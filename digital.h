#pragma once
#include <Arduino.h>

// ===== PINNIT =====
// Mitattavan piirin lähtö: pinni 2 = PE4 = ulkoinen keskeytys INT4 (ks. digital.cpp)
// Saman signaalin jännite:  A0 = ADC0 (ks. adc.h)

// ===== TESTISIGNAALIEN LÄHTÖPINNIT (porttirekisterit, ks. digital.cpp) =====
//   Tuuli   = pinni 32 = PC5
//   Sade    = pinni 33 = PC4
//   Sisä    = pinni 38 = PD7
//   Ulko    = pinni 39 = PG2

// ===== DIGITAALISEN MITTAUKSEN TILAT =====
//   DIG_POIS     digitaalinen tila ei käytössä (päävalikko tai analoginen)
//   DIG_VALINTA  odotetaan kanavan valintaa 1-4
//   DIG_MITTAA   mittaus käynnissä, tulos näytetään 4 s aloituksesta
//   DIG_TULOS    tulos tai vikailmoitus näytöllä: *=uusi, #=valikko
enum DigitaaliTila { DIG_POIS, DIG_VALINTA, DIG_MITTAA, DIG_TULOS };
extern DigitaaliTila digitaali_tila;

void digitalSetup();
void digitalAvaaValinta();              // D painettu: kanavan valinta auki
void digitalAloitaMittaus(int kanava);  // kanava 0-3, mittaus alkaa heti
void digitalKasittele();                // kutsutaan pääsilmukasta
void digitalNollaa();                   // kaikki pois, tila DIG_POIS
