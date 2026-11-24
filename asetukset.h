#pragma once
#include <Arduino.h>

// ===== ASETUKSET-VALIKKO (B) =====
//   1 = Laitteen tiedot   (alanäyttö: versio ja päivityspäivä)
//   2 = Kieli             (alanäyttö: 1=Suomi 2=English, valinta heti)
//   3 = Verkko            (alanäyttö: yhteyden tila)
//   # = takaisin päävalikkoon
//
// Tilat:
//   ASET_POIS    valikko ei ole auki
//   ASET_VALIKKO ylänäytöllä Asetukset, odotetaan 1/2/3 (alanäytöllä voi
//                näkyä laitteen tiedot tai verkon tila)
//   ASET_KIELI   alanäytöllä kielivalinta, odotetaan 1/2
enum AsetusTila { ASET_POIS, ASET_VALIKKO, ASET_KIELI };

void asetuksetAvaa();                 // B painettu päävalikossa
bool asetuksetAktiivinen();
void asetuksetKasittele(char nappi);  // kutsutaan ui.cpp:stä
