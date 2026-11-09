#pragma once
#include <Arduino.h>

// ===== RGB-LEDI (ohjataan suoraan porttirekistereillä, ks. led.cpp) =====
//   Punainen = pinni 3 = PE5
//   Vihreä   = pinni 5 = PE3
//   Sininen  = pinni 6 = PH3

void ledSetup();
void ledVihrea();
void ledKeltainen();
void ledPunainen();
void ledSammuta();
