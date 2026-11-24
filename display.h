#pragma once
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// ===== NÄYTÖT =====
// lcd1 = YLÄNÄYTTÖ: kanavavalinta, tila, ohjeet
// lcd2 = ALANÄYTTÖ: mittaustulokset (analoginen + digitaalinen)
extern LiquidCrystal_I2C lcd1;
extern LiquidCrystal_I2C lcd2;

// ===== LAITTEEN VERSIO =====
// Näkyy käynnistysnäkymässä ja Asetukset > Laitteen tiedot -näkymässä.
// Päivitä molemmat, kun laitteeseen ladataan uusi ohjelmaversio.
#define LAITE_VERSIO     "2.0"
#define LAITE_PAIVITETTY "24.11.26"   // pp.kk.vv

void displaySetup();   // näytöt päälle + käynnistysnäkymä

// ===== YLÄNÄYTÖN OHJAUS =====
void naytaKaynnistys();   // TEST CUBE / Käynnistyy... / Versio, alanäyttö tyhjä
void naytaPaavalikko();   // A/D/B/C + ryhmätunnus, alanäyttö tyhjä

// ===== RYHMÄVALIKKO (C) =====
void naytaRyhmavalikko();                                   // ylänäyttö: 1/2 + #=Valikko
void naytaTunnusSyotto(const char* syotetty, uint8_t pituus); // alanäyttö: syöttö käynnissä
void naytaTunnusTallennettu();                              // alanäyttö: tallennettu + tunnus
void naytaTunnusPoistoVahvistus();                          // alanäyttö: *=Kyllä #=Ei
void naytaTunnusPoistettu();                                // alanäyttö: poistettu!
void naytaEiTunnusta();                                     // alanäyttö: ei poistettavaa

// ===== ASETUKSET (B) =====
void naytaAsetukset();             // ylänäyttö: 1/2/3 + #=Valikko, alanäyttö tyhjä
void naytaLaitteenTiedot();        // alanäyttö: versio ja päivityspäivä
void naytaKielivalinta();          // alanäyttö: 1=Suomi 2=English
void naytaVerkonTila(bool yhteys); // alanäyttö: Yhdistetty / Ei yhteyttä

// ===== DIGITAALINEN (D) =====
void naytaDigitaaliYla(int kanava);        // ylänäyttö: Digitaal. + mittauksen nimi (-1 = ei valittu)
void naytaDigitaalinenValikko();           // alanäyttö: Valitse mittaus 1-4

// ===== ANALOGINEN (A) =====
void naytaAnalogYla(int sensori);          // ylänäyttö: Analoginen + mittaus + kaava (0 = ei valittu)
void naytaAnalogiaValikko();               // alanäyttö: Valitse mittaus 1/2/3
void naytaTestipisteValikko(int sensori);  // alanäyttö: testipisteet 1-5

// ===== DIGITAALINEN MITTAUS (alanäyttö) =====
void naytaDigitaaliMittaus();   // "Mittaus..." mittauksen ajaksi

// Tuuli ja sade: keskimääräinen leveys, hyväksytyt/kaikki, keskimääräinen HIGH-jännite
void naytaDigitaaliPulssiTulos(bool ok, uint32_t keskiLeveysUs, bool onPulsseja,
                               uint16_t hyvaksytyt, uint16_t kaikki, uint16_t jannite_mV);

// Kosteus: tulos kullekin testitaajuudelle (6,9 / 7,4 / 7,9 kHz) ja HIGH-jännite
void naytaDigitaaliKosteusTulos(bool ok, const bool taajuusOk[3], bool onPulsseja,
                                uint16_t jannite_mV);

// ===== ANALOGINEN TESTITULOS =====
void naytaAnalogiTulos(bool ok, int mitattuMV, int odotettuMV);

// ===== VIKATILA =====
void naytaVikatila(const char* viesti);  // Näyttää vikailmoituksen molemmilla näytöillä