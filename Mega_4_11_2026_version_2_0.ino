#include <Keypad.h>
#include "analog.h"
#include "digital.h"
#include "led.h"
#include "display.h"
#include "ui.h"
#include "lang.h"
#include "adc.h"

// ===== SETUP =====
void setup() {
  Serial.begin(9600);

  displaySetup();  // LCD-näytöt
  uiSetup();       // Näppäimistö ja multiplekseri

  adcSetup();      // AD-muunnin rekistereillä (A0, A1)
  analogSetup();   // MBED käynnistys + DAC nollaus
  digitalSetup();  // Rain interrupt
  ledSetup();

  Serial.println(txt("Valmis. A=analoginen  D=digitaalinen  B=kieli  *=nollaa",
                     "Ready.  A=analog      D=digital       B=lang   *=reset"));
  Serial.println(txt("D: 1=tuuli 2=sade 3=sisa 4=ulko",
                     "D: 1=wind  2=rain 3=in   4=out"));
}

// ===== LOOP =====
void loop() {
  // Digitaalinen mittaus: testisignaalin ja mittauksen hoitavat keskeytykset
  // taustalla (digital.cpp). Tämä vain seuraa aikaa ja näyttää tuloksen
  // 4 s kohdalla. Ei odota missään, joten näppäimistö reagoi aina.
  digitalKasittele();

  uiKasitteleNappi();   // Näppäimistö + valikkologiikka
}
