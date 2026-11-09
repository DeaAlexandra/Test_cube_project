#include "adc.h"

// =====================================================================
// ===== ADC:N REKISTERIT ===============================================
// =====================================================================
//  ADMUX   REFS1:0  referenssi: 01 = AVcc (5 V), AREF-pinnissä kondensaattori
//          ADLAR    0 = tulos oikealle tasattuna (10 bittiä: ADCH bitit 1:0, ADCL 7:0)
//          MUX4:0   kanavan alaosa (0 = ADC0, 1 = ADC1 ...)
//  ADCSRB  MUX5     kanavan ylin bitti (kanavat 8-15), tässä 0
//          ADTS2:0  automaattisen käynnistyksen lähde: 000 = free running
//  ADCSRA  ADEN     muunnin päälle
//          ADSC     käynnistä muunnos (lukee 1 niin kauan kuin muunnos kesken)
//          ADATE    automaattinen käynnistys (ADTS:n mukaan)
//          ADIF     muunnos valmis -lippu (nollataan kirjoittamalla 1)
//          ADIE     muunnos valmis -keskeytys sallittu
//          ADPS2:0  esijakaja
//  ADC     tulosrekisteri (ADCL + ADCH). Luettava järjestyksessä ADCL, ADCH;
//          16-bittinen ADC-makro tekee tämän oikein. ADCL:n luku lukitsee
//          tuloksen, kunnes ADCH on luettu, joten tavut ovat samasta muunnoksesta.
//  DIDR0   digitaalisen tulopuskurin esto analogisilta pinneiltä.

volatile uint16_t adcMaksimi   = 0;
volatile uint16_t adcMinimi    = 1023;
volatile bool     adcVaiheHigh = false;

// Perustila: muunnin päällä, tarkka esijakaja, ei keskeytystä eikä jatkuvaa tilaa.
static inline void adcPerustila() {
  ADCSRA = _BV(ADEN) | ADC_JAKAJA_128;
}

// Odottaa mahdollisen meneillään olevan muunnoksen loppuun (enintään 104 µs).
static inline void adcOdotaVapaa() {
  while (ADCSRA & _BV(ADSC)) {}
}

// ===== SETUP =====
void adcSetup() {
  // A0 ja A1 tuloiksi ilman ylösvetovastusta (DDRF/PORTF bitti 0).
  DDRF  &= ~(_BV(PF0) | _BV(PF1));
  PORTF &= ~(_BV(PF0) | _BV(PF1));

  // Digitaalinen tulopuskuri pois A0:lta ja A1:ltä. Analoginen jännite
  // puolivälissä (noin 2,5 V) saisi puskurin vaihtamaan tilaa ja kuluttamaan
  // virtaa sekä aiheuttamaan häiriöitä mittaukseen. Pinnejä ei lueta
  // digitaalisesti, joten puskuria ei tarvita.
  DIDR0 |= _BV(ADC0D) | _BV(ADC1D);

  ADMUX  = _BV(REFS0);         // AVcc-referenssi, kanava 0
  ADCSRB = 0;                  // MUX5 = 0, ADTS = free running
  adcPerustila();

  // Ensimmäinen muunnos ADEN:n jälkeen kestää 25 kelloa (analogiapiirin
  // alustus). Tehdään se tässä valmiiksi ja hylätään tulos.
  ADCSRA |= _BV(ADSC);
  adcOdotaVapaa();
}

// ===== YKSITTÄINEN TARKKA MUUNNOS =====
// Analoginen mittaus ei ole aikakriittinen, joten tuloksen odottaminen
// silmukassa (pollaus) on tässä yksinkertaisin ja luotettavin tapa.
uint16_t adcLueKerran(uint8_t kanava) {
  adcOdotaVapaa();
  ADMUX  = _BV(REFS0) | (kanava & 0x07);   // kanava (0-7) ja AVcc-referenssi
  ADCSRB = 0;
  ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADIF) | ADC_JAKAJA_128;  // ADIF=1 nollaa vanhan lipun
  adcOdotaVapaa();                          // ADSC = 0, kun valmis
  return ADC;
}

// ===== JATKUVA NÄYTTEENOTTO: KÄYNNISTYS JA PYSÄYTYS =====
void adcJatkuvaAloita() {
  adcOdotaVapaa();
  ADMUX  = _BV(REFS0) | ADC_KANAVA_PULSSI;
  ADCSRB = 0;                               // ADTS = 000 -> free running
  adcMaksimi   = 0;
  adcMinimi    = 1023;
  adcVaiheHigh = false;
  // ADATE = jatkuva tila, ADIE = keskeytys, ADSC = ensimmäinen muunnos käyntiin.
  // ADIF = 1 nollaa mahdollisen vanhan lipun.
  ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADIE) | _BV(ADIF) | ADC_JAKAJA_32;
}

// HUOM ADIF-lipusta: lippu nollataan kirjoittamalla siihen 1. Tavallinen
// "ADCSRA &= ~x" lukisi ADIF=1 ja kirjoittaisi sen takaisin, jolloin lippu
// nollautuisi vahingossa. Tässä se nollataan tarkoituksella, mutta muualla
// ADIF on maskattava pois (kirjoitetaan 0 = ei muutosta).
void adcJatkuvaLopeta() {
  ADCSRA = ADCSRA & ~(_BV(ADATE) | _BV(ADIE) | _BV(ADIF));  // jatkuva tila ja keskeytys pois
  adcOdotaVapaa();                                         // viimeinen muunnos loppuun
  adcPerustila();
}

// ===== KESKEYTYSPALVELU: ADC-MUUNNOS VALMIS =====
// Jatkuvassa tilassa uusi muunnos alkaa automaattisesti edellisen
// valmistuttua, eli tämä suoritetaan noin 26 µs:n välein. Kosteuden
// 67 µs:n HIGH-vaiheesta saadaan 2 näytettä, tuulen ja sateen
// millisekuntien pulsseista kymmeniä tai satoja.
//
// Reunalla oleva näyte voi päätyä "väärään" vaiheeseen (näyte otettiin juuri
// ennen reunaa, mutta valmistui sen jälkeen). Se ei haittaa: HIGH-vaiheeseen
// joutunut matala näyte ei kasvata maksimia, eikä LOW-vaiheeseen joutunut
// korkea näyte pienennä minimiä.
ISR(ADC_vect) {
  uint16_t v = ADC;
  if (adcVaiheHigh) {
    if (v > adcMaksimi) adcMaksimi = v;
  } else {
    if (v < adcMinimi) adcMinimi = v;
  }
}
