#pragma once
#include <Arduino.h>

// ===== VERKKO (Ethernet + MQTT) =====
// TYNKÄ: Ethernet-moduulia ei ole vielä kytketty. Toistaiseksi verkko on
// aina "ei yhteyttä". Kun Ethernet toteutetaan, vain verkko.cpp muuttuu,
// eikä valikkojen koodiin tarvitse koskea.
bool verkkoYhdistetty();
