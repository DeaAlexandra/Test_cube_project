#pragma once
#include <Arduino.h>

// =====================================================================
// ===== AD-MUUNNIN (ADC) REKISTERITASOLLA ==============================
// =====================================================================
// ATmega2560:ssa on yksi 10-bittinen AD-muunnin, jonka edessä on
// multiplekseri (kanavat ADC0...ADC15). Tulos on 0...1023, kun
// referenssinä on käyttöjännite AVcc (5 V): 1 askel = 5 V / 1024 = 4,9 mV.
//
// Muunninta käytetään kahdella tavalla:
//  1) Analoginen mittaus (A1): yksittäinen, tarkka muunnos odottamalla.
//  2) Digitaalinen mittaus (A0): nopea jatkuva muunnos (free running) ja
//     keskeytys ISR(ADC_vect), joka kerää signaalin HIGH- ja LOW-tasot.

// ===== KANAVAT =====
#define ADC_KANAVA_PULSSI   0   // A0 = ADC0 = PF0, digitaalipulssin huippujännite
#define ADC_KANAVA_MITTAUS  1   // A1 = ADC1 = PF1, analogisen piirin lähtöjännite

// ===== ESIJAKAJAT (ADCSRA:n bitit ADPS2:0) =====
// ADC tarvitsee oman kellon, joka jaetaan 16 MHz:n kellosta. Täysi 10 bitin
// tarkkuus saavutetaan, kun ADC-kello on 50...200 kHz. Nopeampi kello
// lyhentää muunnosta, mutta heikentää tarkkuutta.
//   /128 -> 125 kHz, muunnos 13 kelloa = 104 µs, täysi tarkkuus
//   /32  -> 500 kHz, muunnos 13 kelloa = 26 µs,  noin 8-9 bitin tarkkuus
#define ADC_JAKAJA_128  (_BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0))   // 111
#define ADC_JAKAJA_32   (_BV(ADPS2) | _BV(ADPS0))                // 101

void     adcSetup();
uint16_t adcLueKerran(uint8_t kanava);   // yksi tarkka muunnos (/128), odottaa tuloksen

// ===== JATKUVA NÄYTTEENOTTO DIGITAALISEEN MITTAUKSEEN =====
// adcJatkuvaAloita() käynnistää muuntimen jatkuvaan tilaan (kanava A0,
// /32 -> uusi näyte noin 26 µs:n välein) ja sallii keskeytyksen.
// Keskeytyspalvelu lajittelee jokaisen näytteen sen mukaan, onko mitattava
// signaali juuri HIGH- vai LOW-vaiheessa:
//   HIGH-vaihe -> adcMaksimi = vaiheen suurin lukema  (tarkistetaan >= 4,5 V)
//   LOW-vaihe  -> adcMinimi  = vaiheen pienin lukema  (tarkistetaan <= 0,5 V)
// Vaiheen kertoo adcVaiheHigh, jota INT4-keskeytys päivittää signaalin
// reunoilla (digital.cpp). INT4 myös nollaa arvot jokaisen vaiheen alussa.
void adcJatkuvaAloita();
void adcJatkuvaLopeta();

// Keskeytysten yhteiset muuttujat. Pääohjelma käyttää niitä vain
// keskeytyksiltä suojattuna (16-bittisiä muuttujia).
extern volatile uint16_t adcMaksimi;
extern volatile uint16_t adcMinimi;
extern volatile bool     adcVaiheHigh;
