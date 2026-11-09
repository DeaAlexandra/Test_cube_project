#pragma once
#include <Arduino.h>

#define MBED_UART Serial2

// ===== DAC-VAKIOARVOT (mV) =====
extern int DAC_LAMPOTILA_OUTA;
extern int DAC_VALO_OUTA;
extern int DAC_TUULISUUNTA_OUTA;
extern const int DAC_TUULISUUNTA_OUTB;

// ===== TOLERANSSIT (mV) =====
extern int TOL_LAMPOTILA;
extern int TOL_VALO;
extern int TOL_TUULISUUNTA;

// ===== TESTIARVOT (mV) =====
extern const int LAMPOTILA_TESTIARVOT[];
extern const int VALO_TESTIARVOT[];
extern const int TUULISUUNTA_TESTIARVOT[];
extern const int LAMPOTILA_MAARA;
extern const int VALO_MAARA;
extern const int TUULISUUNTA_MAARA;

// ===== MITTAUSPINNI =====
// A1 = ADC1, ks. ADC_KANAVA_MITTAUS (adc.h)

// ===== SYÖTTÖTILA =====
enum SyottoTila {
  EI_SYOTTOA,
  ODOTTAA_ANALOGIKANAVA,
  ODOTTAA_TESTIPISTETTA,
  NAYTTAA_TULOSTA
};
extern SyottoTila syotto_tila;
extern int valittu_sensori;

void analogSetup();
void dacSetBoth(int mvA, int mvB);
void dacNollaa();
void analogAloitaValinta();
void analogKasitteleValinta(char nappi);
bool analogSyottoAktiivinen();
