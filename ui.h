#pragma once
#include <Arduino.h>
#include <Keypad.h>

// ===== NÄPPÄIMISTÖ =====
extern Keypad keypad;

// ===== TILAT =====
extern int  valittu_kanava;

void uiSetup();
void uiKasitteleNappi();  // Lukee näppäimistön ja käsittelee napin
void uiResetoi();