#include "digital.h"
#include "led.h"
#include "errormode.h"
#include "display.h"
#include "lang.h"
#include "adc.h"
#include <util/atomic.h>   // ATOMIC_BLOCK: koodilohko, jota keskeytys ei voi katkaista

// =====================================================================
// ===== DIGITAALINEN MITTAUS: YLEISKUVA ================================
// =====================================================================
// Testeri simuloi signaalilaatikkoa: se syöttää testattavalle piirille
// tunnetun testisignaalin ja mittaa, mitä piiri tekee sille.
//
//   Kanava   Testisignaali                     Mitataan        Hyväksytään
//   tuuli    40 ms pulssi / 200 ms, 10 kpl     lähdön leveys   2-6 ms
//   sade     150 ms pulssi / 300 ms, 10 kpl    lähdön leveys   3-50 ms
//   kosteus  50 % neliöaalto, 6,9 / 7,4 /      lähdön taajuus  ±1 %
//            7,9 kHz, kukin 1 s
//
// Kaikilla kanavilla jokaisesta lähtöpulssista tarkistetaan lisäksi
//   HIGH-taso >= 4,5 V ja edeltävä LOW-taso <= 0,5 V.
// Hyväksyntä: tuuli ja sade vähintään 80 %, kosteus vähintään 95 %
// pulsseista täyttää kaikki ehdot.
//
// Mittauksen aikana työ tehdään keskeytyksissä:
//   Timer1 (TIMER1_COMPA)  tuottaa testisignaalin ja laskee lähetetyt pulssit
//   INT4 (pinni 2)         mittaa lähtöpulssien leveyden ja kokoaa tilastot
//   ADC  (A0)              mittaa lähtösignaalin HIGH- ja LOW-tasot
// Pääsilmukka vain seuraa aikaa ja vaihtaa osamittauksesta toiseen.
// Näyttöä ja sarjaporttia ei päivitetä mittauksen aikana, joten ne eivät
// häiritse ajoitusta. Tulos näytetään aina tasan 4 s aloituksesta.

// ===== MITTAUKSEN AJAT JA RAJAT =====
#define KOKONAISAIKA_MS          4000   // aloituksesta tuloksen näyttämiseen
#define VIKATARKISTUS_MS         1000   // jos yhtään lähtöpulssia ei ole tullut -> vika
#define JALKIAIKA_MS             100    // odotus generaattorin jälkeen (viimeinen lähtöpulssi)
#define PULSSI_HYVAKSYNTA_PROS   80     // tuuli ja sade
#define KOSTEUS_HYVAKSYNTA_PROS  95     // kosteus
#define TAAJUUS_TOLERANSSI_PROS  1      // kosteuden taajuus

// Jänniteraja (mV) ADC-lukemaksi: 5000 mV = 1023
#define ADC_MV(mv)  ((uint16_t)(((uint32_t)(mv) * 1023UL) / 5000UL))
#define HIGH_RAJA_ADC  ADC_MV(4500)     // 920 -> "1" vähintään 4,5 V
#define LOW_RAJA_ADC   ADC_MV(500)      // 102 -> "0" enintään 0,5 V

// =====================================================================
// ===== PINNIT ========================================================
// =====================================================================
// Mitattavan piirin lähtö: pinni 2 = portti E, bitti 4 = ulkoinen keskeytys INT4.
// PINx-rekisteristä luetaan pinnin todellinen jännitetaso (1 = HIGH, 0 = LOW).
#define RAIN_BIT     _BV(PE4)
#define RAIN_ON_HIGH (PINE & RAIN_BIT)

// ===== TESTISIGNAALIN LÄHTÖPINNIT (Arduino Mega 2560) =====
//   Kanava  Arduino-pinni  Portti  Bitti
//   0 tuuli 32             C       5   (PC5)
//   1 sade  33             C       4   (PC4)
//   2 sisä  38             D       7   (PD7)
//   3 ulko  39             G       2   (PG2)
// Pinnit ovat eri porteissa, joten jokaisella on oma maskinsa ja porttinsa.
#define PULSSI_TUULI  _BV(PC5)   // PORTC
#define PULSSI_SADE   _BV(PC4)   // PORTC
#define PULSSI_SISA   _BV(PD7)   // PORTD
#define PULSSI_ULKO   _BV(PG2)   // PORTG

// Kaikki pulssipinnit LOW-tilaan. &= ~maski nollaa vain maskin bitit.
static void digitalNollaaPinnit() {
  PORTC &= ~(PULSSI_TUULI | PULSSI_SADE);
  PORTD &= ~PULSSI_SISA;
  PORTG &= ~PULSSI_ULKO;
}

// Yhden pulssipinnin asetus. Vakiomaskien ansiosta kääntäjä tekee jokaisesta
// rivistä yhden sbi/cbi-konekäskyn, jota mikään keskeytys ei voi katkaista.
static inline void asetaPulssiPinni(int8_t kanava, bool high) {
  switch (kanava) {
    case 0: if (high) PORTC |= PULSSI_TUULI; else PORTC &= ~PULSSI_TUULI; break;
    case 1: if (high) PORTC |= PULSSI_SADE;  else PORTC &= ~PULSSI_SADE;  break;
    case 2: if (high) PORTD |= PULSSI_SISA;  else PORTD &= ~PULSSI_SISA;  break;
    case 3: if (high) PORTG |= PULSSI_ULKO;  else PORTG &= ~PULSSI_ULKO;  break;
  }
}

// =====================================================================
// ===== TESTISIGNAALIN ASETUKSET =======================================
// =====================================================================
// Timer1 on 16-bittinen laskuri (0...65535), jota kellotetaan 16 MHz:n
// kellosta esijakajan kautta. Esijakaja valitaan niin, että pisin vaihe
// mahtuu 16 bittiin, mutta tarkkuus on mahdollisimman hyvä:
//   /1   -> tikki 0,0625 µs, pisin väli 4,1 ms   (kosteus)
//   /256 -> tikki 16 µs,     pisin väli 1,05 s   (tuuli, sade)

// TCCR1B:n kellonvalintabitit CS12:CS11:CS10
#define T1_ESIJAKAJA_1    (_BV(CS10))               // 001 = clk/1
#define T1_ESIJAKAJA_256  (_BV(CS12))               // 100 = clk/256

#define TIKIT_256(us)  ((uint32_t)(us) / 16UL)      // 1 tikki = 16 µs

// Kosteuden neliöaalto: puolijakso tikkeinä esijakajalla 1 (pyöristettynä).
//   puolijakso = 16 MHz / (2 * f)
//   6,9 kHz -> 1159 tikkiä = 6902 Hz
//   7,4 kHz -> 1081 tikkiä = 7401 Hz
//   7,9 kHz -> 1013 tikkiä = 7897 Hz
#define PUOLIJAKSO(f)        ((16000000UL + (f)) / (2UL * (f)))
#define JAKSOT_SEKUNNISSA(f) (8000000UL / PUOLIJAKSO(f))   // = todellinen taajuus Hz

// Yksi osamittaus: generaattori tekee täsmälleen 'pulssit' kappaletta
// LOW-vaihetta + HIGH-vaihetta ja pysähtyy sitten itse.
struct Osamittaus {
  uint16_t ocrHigh;        // OCR1A HIGH-vaiheessa (tikit - 1)
  uint16_t ocrLow;         // OCR1A LOW-vaiheessa  (tikit - 1)
  uint8_t  esijakaja;      // TCCR1B:n CS-bitit
  uint16_t pulssit;        // lähetettävien pulssien määrä
  uint32_t minLeveysUs;    // hyväksytty lähtöpulssin leveys
  uint32_t maxLeveysUs;    // (kosteus: ei leveystarkistusta)
};

static const Osamittaus TUULI_MITTAUS = {
  TIKIT_256(40000UL) - 1, TIKIT_256(160000UL) - 1, T1_ESIJAKAJA_256,   // 40 ms / 200 ms
  10, 2000UL, 6000UL };

static const Osamittaus SADE_MITTAUS = {
  TIKIT_256(150000UL) - 1, TIKIT_256(150000UL) - 1, T1_ESIJAKAJA_256,  // 150 ms / 300 ms
  10, 3000UL, 50000UL };

#define KOSTEUS_OSA(f) { PUOLIJAKSO(f) - 1, PUOLIJAKSO(f) - 1, T1_ESIJAKAJA_1, \
                         (uint16_t)JAKSOT_SEKUNNISSA(f), 0UL, 0xFFFFFFFFUL }
static const Osamittaus KOSTEUS_MITTAUKSET[3] = {
  KOSTEUS_OSA(6900UL), KOSTEUS_OSA(7400UL), KOSTEUS_OSA(7900UL) };

static const Osamittaus& haeOsamittaus(int8_t kanava, uint8_t osa) {
  if (kanava == 0) return TUULI_MITTAUS;
  if (kanava == 1) return SADE_MITTAUS;
  return KOSTEUS_MITTAUKSET[osa];
}

// =====================================================================
// ===== TESTISIGNAALIN GENERAATTORI: TIMER1, CTC-TILA + KESKEYTYS =====
// =====================================================================
// CTC-tila (Clear Timer on Compare match, WGM-tila 4):
//   laskuri TCNT1 kasvaa 0, 1, 2 ... ja kun se saavuttaa OCR1A:n arvon,
//   laitteisto nollaa laskurin ja asettaa lipun OCF1A -> keskeytys
//   TIMER1_COMPA. Väli on (OCR1A + 1) tikkiä.
//
// Signaali alkaa LOW-vaiheella, jotta piirin lähdön LOW-taso ehditään
// mitata ennen ensimmäistä pulssia. Jokaisessa keskeytyksessä pinni
// käännetään ja OCR1A:han ladataan seuraavan vaiheen pituus. Kun viimeisen
// pulssin HIGH-vaihe päättyy, keskeytys pysäyttää ajastimen itse.

static volatile int8_t   pulssiKanava       = -1;     // -1 = generaattori pois
static volatile bool     pulssiHigh         = false;  // kumpi vaihe on menossa
static volatile uint16_t pulssiOcrHigh      = 0;
static volatile uint16_t pulssiOcrLow       = 0;
static volatile uint16_t pulssejaJaljella   = 0;
static volatile bool     generaattoriValmis = false;

// ===== KESKEYTYSPALVELU: TIMER1 COMPARE MATCH A =====
ISR(TIMER1_COMPA_vect) {
  int8_t k = pulssiKanava;
  if (pulssiHigh) {
    // HIGH-vaihe päättyy
    asetaPulssiPinni(k, false);
    pulssiHigh = false;
    if (--pulssejaJaljella == 0) {
      TCCR1B = 0;                  // viimeinen pulssi tehty: kello pois
      TIMSK1 &= ~_BV(OCIE1A);      // ja keskeytys pois
      generaattoriValmis = true;
    } else {
      OCR1A = pulssiOcrLow;
    }
  } else {
    // LOW-vaihe päättyy -> uusi pulssi alkaa
    asetaPulssiPinni(k, true);
    OCR1A = pulssiOcrHigh;
    pulssiHigh = true;
  }
}

// ===== GENERAATTORIN PYSÄYTYS =====
static void generaattoriPysayta() {
  TCCR1B = 0;                  // CS-bitit 000 -> ajastimen kello pois, laskenta seis
  TIMSK1 &= ~_BV(OCIE1A);      // compare A -keskeytys pois
  TIFR1  = _BV(OCF1A);         // mahdollinen odottava lippu pois (kirjoitetaan 1)
  pulssiKanava = -1;
  pulssiHigh   = false;
  digitalNollaaPinnit();
}

// ===== GENERAATTORIN KÄYNNISTYS =====
// Kaikki asetetaan ajastimen seistessä, joten keskeytys ei voi lukea
// puoliksi päivitettyjä arvoja.
static void generaattoriKaynnista(int8_t kanava, const Osamittaus& o) {
  generaattoriPysayta();
  pulssiOcrHigh      = o.ocrHigh;
  pulssiOcrLow       = o.ocrLow;
  pulssejaJaljella   = o.pulssit;
  generaattoriValmis = false;
  pulssiKanava       = kanava;
  pulssiHigh         = false;      // aloitetaan LOW-vaiheella

  // TCCR1A: WGM11:WGM10 = 00 ja COM1x = 00 -> ajastin ei ohjaa OC1x-pinnejä
  //         itse (testipinnit eivät ole Timer1:n lähtöpinnejä, siksi ISR).
  // TCCR1B: WGM13:WGM12 = 01 -> yhdessä edellisen kanssa WGM = 0100 = CTC.
  TCCR1A = 0;
  TCCR1B = _BV(WGM12);         // CTC, kello vielä pois
  TCNT1  = 0;                  // laskuri alusta
  OCR1A  = o.ocrLow;           // ensimmäinen vaihe on LOW

  TIFR1  = _BV(OCF1A);         // vanha lippu pois
  TIMSK1 |= _BV(OCIE1A);       // compare A -keskeytys päälle
  TCCR1B |= o.esijakaja;       // kello päälle -> laskenta alkaa
}

// =====================================================================
// ===== LÄHTÖSIGNAALIN MITTAUS: INT4-KESKEYTYS =========================
// =====================================================================
// Keskeytys kokoaa tilastot suoraan, joten yksikään pulssi ei jää
// laskematta, vaikka pääsilmukka olisi hetken muualla.
// volatile: kääntäjä lukee arvon aina muistista, koska keskeytys voi muuttaa
// sen milloin tahansa. static: muuttujat näkyvät vain tässä tiedostossa.
static volatile bool          mittausKaynnissa = false;  // INT4 kerää tilastoja
static volatile bool          pulssiKesken     = false;  // lähtö on nyt HIGH
static volatile bool          edellinenLowOk   = false;  // LOW-vaihe ennen pulssia
static volatile unsigned long nousuAika        = 0;      // nousevan reunan aika (µs)
static volatile uint32_t      minLeveys        = 0;      // osamittauksen rajat
static volatile uint32_t      maxLeveys        = 0;
static volatile uint16_t      rxPulssit        = 0;      // vastaanotetut pulssit
static volatile uint16_t      rxOk             = 0;      // ehdot täyttäneet pulssit
static volatile uint32_t      leveysSumma      = 0;      // µs, keskiarvoa varten
static volatile uint32_t      huippuSumma      = 0;      // ADC, keskiarvoa varten

// ===== KESKEYTYSPALVELU: ULKOINEN KESKEYTYS INT4 (pinni 2) =====
// Laitteisto kutsuu tätä jokaisella pinnin 2 tilanmuutoksella.
// Palvelu pidetään lyhyenä: vain aikaleimat ja laskurit, ei tulostusta.
ISR(INT4_vect) {
  if (!mittausKaynnissa) return;

  if (RAIN_ON_HIGH) {
    if (pulssiKesken) return;
    // Nouseva reuna: LOW-vaihe päättyi -> sen pienin jännite talteen,
    // ADC kerää tästä eteenpäin HIGH-vaiheen suurinta jännitettä.
    nousuAika      = micros();
    edellinenLowOk = (adcMinimi <= LOW_RAJA_ADC);
    adcMaksimi     = 0;
    adcVaiheHigh   = true;
    pulssiKesken   = true;
  } else if (pulssiKesken) {
    // Laskeva reuna: pulssi valmis -> arvioidaan se heti.
    uint32_t leveys = micros() - nousuAika;
    uint16_t huippu = adcMaksimi;
    adcVaiheHigh = false;
    adcMinimi    = 1023;
    pulssiKesken = false;

    rxPulssit++;
    leveysSumma += leveys;
    huippuSumma += huippu;
    if (leveys >= minLeveys && leveys <= maxLeveys &&
        huippu >= HIGH_RAJA_ADC && edellinenLowOk) {
      rxOk++;
    }
  }
  // Laskeva reuna ilman edeltävää nousevaa reunaa jätetään huomiotta.
}

// =====================================================================
// ===== MITTAUKSEN KULKU (PÄÄSILMUKKA) =================================
// =====================================================================
struct OsaTulos {
  uint16_t lahetetyt;     // generaattorin pulssit
  uint16_t vastaanotetut; // piirin lähtöpulssit
  uint16_t hyvaksytyt;    // ehdot täyttäneet lähtöpulssit
  uint32_t leveysSumma;
  uint32_t huippuSumma;
};

DigitaaliTila digitaali_tila = DIG_POIS;

static int8_t        mitattavaKanava   = -1;
static uint8_t       osaIndeksi        = 0;
static uint8_t       osaMaara          = 0;
static unsigned long mittausAlkuMs     = 0;
static unsigned long generaattoriLoppuMs = 0;
static bool          loppuHavaittu     = false;
static bool          vikaTarkistettu   = false;
static bool          kaikkiValmis      = false;
static OsaTulos      tulokset[3];

// ===== OSAMITTAUKSEN ALOITUS =====
static void aloitaOsamittaus(uint8_t osa) {
  const Osamittaus& o = haeOsamittaus(mitattavaKanava, osa);

  // Tilastot nollataan keskeytyksiltä suojattuna (32-bittisiä muuttujia).
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    rxPulssit    = 0;
    rxOk         = 0;
    leveysSumma  = 0;
    huippuSumma  = 0;
    pulssiKesken = false;
    minLeveys    = o.minLeveysUs;
    maxLeveys    = o.maxLeveysUs;
  }
  loppuHavaittu = false;

  adcJatkuvaAloita();                       // ADC mittaa jo ennen ensimmäistä pulssia
  mittausKaynnissa = true;                  // INT4 alkaa koota tilastoja
  generaattoriKaynnista(mitattavaKanava, o);
}

// ===== OSAMITTAUKSEN LOPETUS =====
static void lopetaOsamittaus(uint8_t osa) {
  mittausKaynnissa = false;
  generaattoriPysayta();
  adcJatkuvaLopeta();

  OsaTulos& t = tulokset[osa];
  t.lahetetyt = haeOsamittaus(mitattavaKanava, osa).pulssit;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    t.vastaanotetut = rxPulssit;
    t.hyvaksytyt    = rxOk;
    t.leveysSumma   = leveysSumma;
    t.huippuSumma   = huippuSumma;
  }
}

// ===== KAIKKI POIS =====
static void pysaytaKaikki() {
  mittausKaynnissa = false;
  generaattoriPysayta();
  adcJatkuvaLopeta();
}

// ===== APUFUNKTIOT TULOKSELLE =====
static uint16_t adcMillivolteiksi(uint32_t adc) {
  return (uint16_t)((adc * 5000UL) / 1023UL);
}

// Hyväksytty, jos ehdot täyttäneitä pulsseja on vähintään 'prosentti' %.
// Jakajana on suurempi lähetetyistä ja vastaanotetuista: jos piiri
// värähtelee ja tuottaa ylimääräisiä pulsseja, osuus pienenee.
static bool riittavastiHyvaksyttyja(const OsaTulos& t, uint8_t prosentti, uint16_t* jakaja) {
  uint16_t j = max(t.lahetetyt, t.vastaanotetut);
  if (jakaja) *jakaja = j;
  if (t.vastaanotetut == 0) return false;
  return (uint32_t)t.hyvaksytyt * 100UL >= (uint32_t)prosentti * j;
}

// ===== TULOKSEN ARVIOINTI JA NÄYTTÄMINEN =====
static void naytaTulos() {
  bool ok;

  if (mitattavaKanava <= 1) {
    // ----- Tuuli ja sade: pulssin leveys -----
    const OsaTulos& t = tulokset[0];
    uint16_t jakaja;
    ok = riittavastiHyvaksyttyja(t, PULSSI_HYVAKSYNTA_PROS, &jakaja);
    uint32_t keskiLeveys = t.vastaanotetut ? t.leveysSumma / t.vastaanotetut : 0;
    uint16_t keskiMv     = t.vastaanotetut ? adcMillivolteiksi(t.huippuSumma / t.vastaanotetut) : 0;

    Serial.print(txt("Leveys ka: ", "Width avg: "));
    Serial.print(keskiLeveys); Serial.print(" us  U ka: ");
    Serial.print(keskiMv); Serial.print(" mV  ");
    Serial.print(t.hyvaksytyt); Serial.print("/"); Serial.println(jakaja);

    naytaDigitaaliPulssiTulos(ok, keskiLeveys, t.vastaanotetut > 0,
                              t.hyvaksytyt, jakaja, keskiMv);
  } else {
    // ----- Kosteus: taajuus kolmella testitaajuudella -----
    bool taajuusOk[3];
    uint32_t huippuYht = 0, pulssitYht = 0;
    ok = true;
    for (uint8_t i = 0; i < 3; i++) {
      const OsaTulos& t = tulokset[i];
      // Lähetettyjä jaksoja on täsmälleen 1 sekunnin verran, joten
      // vastaanotettujen pulssien määrä on suoraan lähdön taajuus (Hz).
      uint16_t ero      = (t.vastaanotetut > t.lahetetyt) ? t.vastaanotetut - t.lahetetyt
                                                          : t.lahetetyt - t.vastaanotetut;
      bool fOk          = (uint32_t)ero * 100UL <= (uint32_t)TAAJUUS_TOLERANSSI_PROS * t.lahetetyt;
      bool tasotOk      = riittavastiHyvaksyttyja(t, KOSTEUS_HYVAKSYNTA_PROS, nullptr);
      taajuusOk[i]      = fOk && tasotOk;
      ok               &= taajuusOk[i];
      huippuYht        += t.huippuSumma;
      pulssitYht       += t.vastaanotetut;

      Serial.print("f="); Serial.print(t.lahetetyt);
      Serial.print(" Hz -> "); Serial.print(t.vastaanotetut);
      Serial.print(txt(" Hz, tasot ok ", " Hz, levels ok "));
      Serial.print(t.hyvaksytyt); Serial.print("  ");
      Serial.println(taajuusOk[i] ? "OK" : txt("VIRHE", "ERROR"));
    }
    uint16_t keskiMv = pulssitYht ? adcMillivolteiksi(huippuYht / pulssitYht) : 0;
    naytaDigitaaliKosteusTulos(ok, taajuusOk, pulssitYht > 0, keskiMv);
  }

  Serial.println(ok ? "OK" : txt("VIRHE", "ERROR"));
  if (ok) ledVihrea();
  else    ledPunainen();
  digitaali_tila = DIG_TULOS;
}

// ===== VIKA: EI LÄHTÖPULSSEJA =====
// Mittaus keskeytetään heti, ei jäädä odottamaan. Lähdön tasajännite
// kertoo vian tyypin: 0 V = oikosulku, 5 V = avoin piiri / jumissa
// ylhäällä, muu = ei signaalia.
static void keskeytaVikaan() {
  pysaytaKaikki();
  uint16_t taso   = adcLueKerran(ADC_KANAVA_PULSSI);
  VikaTyyppi vika = tarkistaAdcVika(taso);
  if (vika == VIKA_EI_VIKAA) vika = VIKA_EI_SIGNAALIA;
  Serial.print(txt("Vika, lahdon taso ADC=", "Fault, output level ADC="));
  Serial.println(taso);
  kasitteleVikatila(vika);       // ilmoitus näytöille ja punainen ledi
  digitaali_tila = DIG_TULOS;    // *=uusi, #=valikko
}

// =====================================================================
// ===== JULKISET FUNKTIOT ==============================================
// =====================================================================
void digitalSetup() {
  // Mitattavan piirin lähtö tuloksi: DDR-bitti 0. PORT-bitti 0 = ei sisäistä
  // ylösvetovastusta (sama kuin pinMode(INPUT)).
  DDRE  &= ~RAIN_BIT;
  PORTE &= ~RAIN_BIT;

  // Testipinnit lähdöiksi (DDR-bitti 1) ja varmuuden vuoksi LOW-tilaan.
  // generaattoriPysayta() myös ottaa Timer1:n pois Arduinon oletusasetuksista
  // (init() asettaa sen PWM-tilaan analogWrite-käyttöä varten).
  generaattoriPysayta();
  DDRC |= PULSSI_TUULI | PULSSI_SADE;
  DDRD |= PULSSI_SISA;
  DDRG |= PULSSI_ULKO;

  // Ulkoinen keskeytys INT4 rekistereillä:
  //  EIMSK (External Interrupt Mask): bitti INT4 = keskeytys sallittu.
  //  EICRB (External Interrupt Control B): kaksi bittiä ISC41:ISC40 valitsevat,
  //        mikä tapahtuma laukaisee keskeytyksen INT4:
  //          00 = LOW-taso, 01 = mikä tahansa muutos,
  //          10 = laskeva reuna, 11 = nouseva reuna
  //  EIFR  (External Interrupt Flag): odottava keskeytys. Lippu nollataan
  //        kirjoittamalla siihen 1 (AVR:n tapa).
  EIMSK &= ~_BV(INT4);                                   // pois asetusten ajaksi
  EICRB  = (EICRB & ~(_BV(ISC41) | _BV(ISC40)))          // nollaa INT4:n bitit
           | _BV(ISC40);                                 // 01 = mikä tahansa muutos
  EIFR   = _BV(INTF4);                                   // vanha lippu pois
  EIMSK |= _BV(INT4);                                    // keskeytys päälle
  // Globaalit keskeytykset (SREG:n I-bitti) Arduino on jo sallinut ennen setup():ia.
}

void digitalAvaaValinta() {
  digitalNollaa();
  digitaali_tila = DIG_VALINTA;
}

void digitalAloitaMittaus(int kanava) {
  pysaytaKaikki();
  if (kanava < 0 || kanava > 3) return;

  mitattavaKanava = kanava;
  osaMaara        = (kanava <= 1) ? 1 : 3;
  osaIndeksi      = 0;
  kaikkiValmis    = false;
  vikaTarkistettu = false;

  ledSammuta();
  naytaDigitaaliMittaus();
  Serial.println(txt("Mittaus alkaa...", "Measuring..."));

  mittausAlkuMs  = millis();
  digitaali_tila = DIG_MITTAA;
  aloitaOsamittaus(0);
}

// Kutsutaan pääsilmukasta. Ei odota missään, vaan tarkistaa ajan ja palaa.
void digitalKasittele() {
  if (digitaali_tila != DIG_MITTAA) return;
  unsigned long kulunut = millis() - mittausAlkuMs;

  if (!kaikkiValmis) {
    // 1) Vikatarkistus kerran: onko piiri vastannut ensimmäisen sekunnin aikana?
    if (!vikaTarkistettu && kulunut >= VIKATARKISTUS_MS) {
      vikaTarkistettu = true;
      uint16_t rx;
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { rx = rxPulssit; }
      if (osaIndeksi == 0 && rx == 0) {
        keskeytaVikaan();
        return;
      }
    }

    // 2) Kun generaattori on lähettänyt kaikki pulssit, odotetaan vielä
    //    jälkiaika (piirin viimeinen lähtöpulssi) ja siirrytään seuraavaan.
    if (generaattoriValmis) {
      if (!loppuHavaittu) {
        loppuHavaittu       = true;
        generaattoriLoppuMs = millis();
      } else if (millis() - generaattoriLoppuMs >= JALKIAIKA_MS) {
        lopetaOsamittaus(osaIndeksi);
        osaIndeksi++;
        if (osaIndeksi < osaMaara) aloitaOsamittaus(osaIndeksi);
        else                       kaikkiValmis = true;
      }
    }
  }

  // 3) Tulos näytetään aina samaan aikaan aloituksesta.
  if (kaikkiValmis && kulunut >= KOKONAISAIKA_MS) {
    naytaTulos();
  }
}

void digitalNollaa() {
  pysaytaKaikki();               // Timer1 ja ADC seis, testipinnit LOW
  mitattavaKanava = -1;
  digitaali_tila  = DIG_POIS;
}
