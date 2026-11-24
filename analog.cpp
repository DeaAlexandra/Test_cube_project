#include "analog.h"
#include "led.h"
#include "errormode.h"
#include "display.h"
#include "lang.h"
#include "adc.h"
#include "ryhma.h"
#include "tietokanta.h"
#include "ui.h"           // keypad: tietokantamittauksen keskeytys

// ===== DAC-VAKIOARVOT (mV) =====
int DAC_LAMPOTILA_OUTA   = 1200;
int DAC_VALO_OUTA        = 1500;
int DAC_TUULISUUNTA_OUTA = 3200;
const int DAC_TUULISUUNTA_OUTB = 1200;

// ===== TOLERANSSIT (mV) =====
int TOL_LAMPOTILA   = 100;
int TOL_VALO        = 100;
int TOL_TUULISUUNTA = 100;

// ===== TESTIARVOT (mV) =====
const int LAMPOTILA_TESTIARVOT[]   = {500, 800, 1200, 1500, 1650};
const int VALO_TESTIARVOT[]        = {620, 900, 1200, 1800, 2250};
const int TUULISUUNTA_TESTIARVOT[] = {1700, 2200, 2700, 3200, 3900};
const int LAMPOTILA_MAARA   = 5;
const int VALO_MAARA        = 5;
const int TUULISUUNTA_MAARA = 5;

// ===== TILA =====
SyottoTila syotto_tila    = EI_SYOTTOA;
int        valittu_sensori = -1;

// true, kun alanäytöllä on tietokantamittauksen tulos tai lähetysnäkymä:
// vain silloin nappi 0 lähettää. Yksittäisen testin tuloksesta (1-5) ei
// lähetetä vanhaa odottavaa sarjaa vahingossa.
static bool dbTulosNaytolla = false;

// ===== SETUP =====
void analogSetup() {
  MBED_UART.begin(9600);
  // Mittauspinni A1 asetetaan tuloksi adcSetup():ssa (adc.cpp).
  delay(2000);
  dacNollaa();
}

// ===== DAC =====
void dacSetBoth(int mvA, int mvB) {
  int arvoA = constrain(mvA, 0, 4095);
  int arvoB = constrain(mvB, 0, 4095);
  Serial.print("[DAC->Bob] AB");
  Serial.print(arvoA); Serial.print(","); Serial.println(arvoB);
  MBED_UART.print("AB");
  MBED_UART.print(arvoA); MBED_UART.print(",");
  MBED_UART.print(arvoB); MBED_UART.print("\n");
}

void dacNollaa() {
  Serial.println("[DAC->Bob] R");
  MBED_UART.print("R\n");
}

// ===== ODOTUS, JONKA VOI KESKEYTTÄÄ =====
// Odottaa 'ms' millisekuntia ja lukee samalla näppäimistöä. Palauttaa false,
// jos käyttäjä painoi '#' (tietokantamittauksen keskeytys).
static bool odotaKeskeytettavasti(unsigned long ms) {
  unsigned long alku = millis();
  while (millis() - alku < ms) {
    if (keypad.getKey() == '#') return false;
  }
  return true;
}

// ===== MITTAUS: 1s vakautus + keskiarvo 16 lukemasta =====
// Keskiarvo vähentää satunnaista kohinaa: 16 näytteen keskiarvossa kohina
// pienenee noin neljäsosaan (neliöjuuri 16:sta).
// keskeytettava = true: vakautusaikana '#' keskeyttää, paluuarvo -1.
static int mittaaADC(bool keskeytettava) {
  if (keskeytettava) {
    if (!odotaKeskeytettavasti(1000)) return -1;
  } else {
    delay(1000);
  }
  long summa = 0;
  for (int k = 0; k < 16; k++) {
    summa += adcLueKerran(ADC_KANAVA_MITTAUS);
    delay(10);
  }
  return (int)(summa / 16);
}

// ===== MITTAUKSEN PARAMETRIT =====
struct AnalogParametrit {
  const int* arvot;       // testipisteiden Uin (mV)
  int        dacB;        // kanava B (mV), tuulen suunnan 1.2 V
  float      kerroin;     // Uout = kerroin * Uin - dacB
  int        toleranssi;  // mV
};

static bool haeParametrit(int sensori, AnalogParametrit& p) {
  switch (sensori) {
    case 1: p = { LAMPOTILA_TESTIARVOT,   0,                    3.0, TOL_LAMPOTILA   }; return true;
    case 2: p = { VALO_TESTIARVOT,        0,                    2.0, TOL_VALO        }; return true;
    case 3: p = { TUULISUUNTA_TESTIARVOT, DAC_TUULISUUNTA_OUTB, 1.0, TOL_TUULISUUNTA }; return true;
    default: return false;
  }
}

// ===== YHDEN PISTEEN MITTAUS =====
// Asettaa DAC:n, mittaa ja vertaa. Ei koske näyttöön, joten samaa funktiota
// käyttävät sekä yksittäinen testi että tietokantamittaus.
enum PisteTila { PISTE_MITATTU, PISTE_VIKA, PISTE_KESKEYTETTY };

struct PisteTulos {
  int        syottoMV;
  int        odotettuMV;
  int        mitattuMV;
  bool       ok;
  VikaTyyppi vika;
};

static PisteTila mittaaPiste(int sensori, int pisteIndeksi, bool keskeytettava,
                             PisteTulos& t) {
  AnalogParametrit p;
  if (!haeParametrit(sensori, p)) return PISTE_KESKEYTETTY;

  t.syottoMV   = p.arvot[pisteIndeksi];
  t.odotettuMV = (int)(t.syottoMV * p.kerroin) - p.dacB;
  t.mitattuMV  = 0;
  t.ok         = false;
  t.vika       = VIKA_EI_VIKAA;

  Serial.print(TXT("Piste ", "Point "));
  Serial.print(pisteIndeksi + 1);
  Serial.print(F(": Uin="));
  Serial.print(t.syottoMV);
  Serial.println(F(" mV"));

  dacSetBoth(t.syottoMV, p.dacB);

  int adcArvo = mittaaADC(keskeytettava);
  if (adcArvo < 0) return PISTE_KESKEYTETTY;
  Serial.print(F("[ADC] ")); Serial.println(adcArvo);

  t.vika = tarkistaAdcVika(adcArvo);
  if (t.vika != VIKA_EI_VIKAA) return PISTE_VIKA;

  t.mitattuMV = (int)(adcArvo * (5000.0 / 1023.0));
  t.ok        = abs(t.mitattuMV - t.odotettuMV) <= p.toleranssi;

  Serial.print(TXT("Mitattu: ", "Measured: "));
  Serial.print(t.mitattuMV);
  Serial.print(TXT(" mV  Odotettu: ", " mV  Expected: "));
  Serial.print(t.odotettuMV);
  Serial.print(F(" mV  (+-"));
  Serial.print(p.toleranssi);
  Serial.print(F(" mV)  "));
  Serial.println(t.ok ? F("OK") : TXT("VIRHE", "ERROR"));
  return PISTE_MITATTU;
}

// ===== YKSITTÄINEN TESTI (napit 1-5) =====
static void suoritaTesti(int sensori, int pisteIndeksi) {
  PisteTulos t;
  PisteTila  tila = mittaaPiste(sensori, pisteIndeksi, false, t);

  if (tila == PISTE_VIKA) {
    dacNollaa();
    kasitteleVikatila(t.vika);       // ilmoitus + punainen ledi, ei pysäytä
    syotto_tila = NAYTTAA_TULOSTA;   // *=uusi testi, #=valikko
    return;
  }
  if (tila != PISTE_MITATTU) return;

  if (t.ok) ledVihrea();
  else      ledPunainen();

  naytaAnalogiTulos(t.ok, t.mitattuMV, t.odotettuMV);
  syotto_tila = NAYTTAA_TULOSTA;
}

// ===== NAVIGOINTI =====
static void paluuPaavalikkoon() {
  dacNollaa();
  vikatilaNollaa();
  ledSammuta();
  syotto_tila     = EI_SYOTTOA;
  valittu_sensori = -1;
  naytaPaavalikko();
}

// ===== TIETOKANTAMITTAUS (nappi 0) =====
// Mittaa kaikki 5 testipistettä peräkkäin (noin 6 s) ja tallentaa tuloksen
// työmuistiin odottamaan lähetystä. Vaatii ryhmätunnuksen.
//   '#' mittauksen aikana keskeyttää ja palaa päävalikkoon
//   vika (oikosulku tms.) keskeyttää ja näyttää vikatilan
static void suoritaTietokantamittaus(int sensori) {
  dbTulosNaytolla = false;           // tosi vasta, kun tulos on näytöllä
  if (!ryhmaTunnusAsetettu()) {
    Serial.println(TXT("Ryhmatunnus puuttuu (C-valikko)", "Group ID missing (C menu)"));
    naytaRyhmatunnusPuuttuu();
    syotto_tila = NAYTTAA_TULOSTA;   // *=takaisin testipisteisiin, #=valikko
    return;
  }

  tietokantaTyhjenna();              // edellinen odottava tulos korvataan
  ledSammuta();
  Serial.println(TXT("TIETOKANTAMITTAUS", "DATABASE TEST"));

  AnalogTietokantaTulos& db = analogDbTulos;
  db.sensori = sensori;
  int8_t ensimmainenVirhe = -1;      // 1-5, -1 = ei virheitä

  for (uint8_t i = 0; i < ANALOG_DB_PISTEET; i++) {
    naytaTietokantamittausKaynnissa(i + 1, ANALOG_DB_PISTEET);

    PisteTulos t;
    PisteTila  tila = mittaaPiste(sensori, i, true, t);

    if (tila == PISTE_KESKEYTETTY) {
      Serial.println(TXT("Keskeytetty", "Aborted"));
      tietokantaTyhjenna();
      paluuPaavalikkoon();
      return;
    }
    if (tila == PISTE_VIKA) {
      tietokantaTyhjenna();          // vajaata tulosta ei lähetetä
      dacNollaa();
      kasitteleVikatila(t.vika);
      syotto_tila = NAYTTAA_TULOSTA;
      return;
    }

    db.syotto_mV[i]   = t.syottoMV;
    db.odotettu_mV[i] = t.odotettuMV;
    db.mitattu_mV[i]  = t.mitattuMV;
    db.ok[i]          = t.ok;
    if (t.ok) db.hyvaksytyt++;
    else if (ensimmainenVirhe < 0) ensimmainenVirhe = i + 1;
  }

  dacNollaa();
  db_odottaa = DB_ANALOG;            // odottaa lähetystä (vain hyväksytty lähetetään)
  bool kaikkiOk = (db.hyvaksytyt == ANALOG_DB_PISTEET);

  Serial.print(TXT("Pisteet: ", "Points: "));
  Serial.print(db.hyvaksytyt);
  Serial.print(F("/"));
  Serial.println(ANALOG_DB_PISTEET);

  if (kaikkiOk) ledVihrea();
  else          ledPunainen();

  naytaTietokantaTulos(db.hyvaksytyt, ANALOG_DB_PISTEET, ensimmainenVirhe);
  syotto_tila     = NAYTTAA_TULOSTA;
  dbTulosNaytolla = true;
}

void analogAloitaValinta() {
  syotto_tila = ODOTTAA_ANALOGIKANAVA;
  Serial.println(txt("ANALOGINEN VALINTA:", "ANALOG SELECTION:"));
  Serial.println(txt("1=lampotila  2=valo  3=tuulisuunta",
                     "1=temp  2=light  3=wind dir"));
  Serial.println(txt("#=valikko", "#=menu"));
}

void analogKasitteleValinta(char nappi) {
  if (nappi == '#') { dbTulosNaytolla = false; paluuPaavalikkoon(); return; }

  if (syotto_tila == NAYTTAA_TULOSTA) {
    if (nappi == '*') {
      // Vikailmoitus oli vain alanäytöllä, ylänäyttö on ennallaan.
      if (vikatila_aktiivinen) vikatilaNollaa();
      ledSammuta();
      dbTulosNaytolla = false;
      syotto_tila = ODOTTAA_TESTIPISTETTA;
      naytaTestipisteValikko(valittu_sensori);
    } else if (nappi == '0' && dbTulosNaytolla) {
      tietokantaLaheta();            // 0=Lähetä / 0=Yritä uudelleen
    }
    return;
  }

  if (syotto_tila == ODOTTAA_ANALOGIKANAVA && nappi >= '1' && nappi <= '3') {
    valittu_sensori = nappi - '0';
    syotto_tila = ODOTTAA_TESTIPISTETTA;
    naytaAnalogYla(valittu_sensori);          // mittauksen nimi ja kaava ylös
    naytaTestipisteValikko(valittu_sensori);
    return;
  }

  if (syotto_tila == ODOTTAA_TESTIPISTETTA) {
    if (nappi >= '1' && nappi <= '5') suoritaTesti(valittu_sensori, nappi - '1');
    else if (nappi == '0')            suoritaTietokantamittaus(valittu_sensori);
  }
}

bool analogSyottoAktiivinen() {
  return syotto_tila != EI_SYOTTOA;
}
