#pragma once
#include <Arduino.h>

// ===== VERKKO (Ethernet + MQTT) =====
// TYNKÄ: Ethernet-moduulia ei ole vielä kytketty. Kun Ethernet toteutetaan,
// vain verkko.cpp muuttuu, eikä valikkojen koodiin tarvitse koskea.

// Lähetyksen simulointi tyngässä (vain testaukseen):
//   1 = lähetys "onnistuu": näkyy Lähetetty!
//   0 = lähetys epäonnistuu: näkyy Lähetys epäonnistui / 0=Yritä uudelleen
// JSON tulostuu sarjamonitoriin kummassakin tapauksessa.
#define VERKKO_SIMULOI_ONNISTUMINEN 1

bool verkkoYhdistetty();

// Lähettää odottavan tietokantatuloksen (tietokanta.h). true = onnistui.
bool verkkoLahetaTulos();
