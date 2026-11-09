#pragma once
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// ===== NÄYTÖT =====
// lcd1 = YLÄNÄYTTÖ: kanavavalinta, tila, ohjeet
// lcd2 = ALANÄYTTÖ: mittaustulokset (analoginen + digitaalinen)
extern LiquidCrystal_I2C lcd1;
extern LiquidCrystal_I2C lcd2;

void displaySetup();

// ===== YLÄNÄYTÖN OHJAUS =====
void naytaTervetuloviesti();
void naytaDigitaalinenValikko();
void naytaAnalogiaValikko();
void naytaTestipisteValikko(int sensori);  // Näyttää testipisteet sensorin mukaan
void naytaKanava(int kanava);

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