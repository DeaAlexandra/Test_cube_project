#pragma once
#include <Arduino.h>

// ===== RYHMÄTUNNUS =====
// Muoto 26A1234: vuosikurssi (2 numeroa), luokka (kirjain A-D), 4 numeroa.
// Tunnus on VAIN työmuistissa: virran katketessa se katoaa tarkoituksella,
// jotta seuraavan ryhmän mittaukset eivät mene edellisen nimiin.
// Mittaukset toimivat ilman tunnusta, tunnus tarvitaan vain tietokantaan.
#define RYHMATUNNUS_PITUUS 7

extern char ryhmatunnus[RYHMATUNNUS_PITUUS + 1];   // "" = ei asetettu

bool ryhmaTunnusAsetettu();

// Kirjoittaa näytön tekstin "R=26A1234" tai "R=-------" (vähintään 10 tavua).
void ryhmaTunnusNaytolle(char* buf, size_t koko);

// ===== RYHMÄVALIKKO (C) =====
//   1 = Syötä tunnus   vaihe vaiheelta: 2 numeroa, kirjain A-D (napit A-D),
//                      4 numeroa. Tallentuu, kun 7. merkki on syötetty.
//                      *=poista merkki, #=keskeytä (vanha tunnus säilyy)
//   2 = Poista tunnus  vahvistus *=Kyllä #=Ei
//   # = takaisin päävalikkoon
//
// Tilat:
//   RYHMA_POIS      valikko ei ole auki
//   RYHMA_VALIKKO   odotetaan 1/2 (alanäytöllä voi näkyä edellisen toiminnon tulos)
//   RYHMA_SYOTTO    tunnuksen syöttö käynnissä
//   RYHMA_POISTO    odotetaan poiston vahvistusta
enum RyhmaTila { RYHMA_POIS, RYHMA_VALIKKO, RYHMA_SYOTTO, RYHMA_POISTO };

void ryhmaAvaa();                  // C painettu päävalikossa
bool ryhmaAktiivinen();
void ryhmaKasittele(char nappi);   // kutsutaan ui.cpp:stä
