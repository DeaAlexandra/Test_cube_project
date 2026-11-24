#pragma once
#include <Arduino.h>

// ===== RYHMÄTUNNUS =====
// Muoto 26A1234: vuosikurssi (2 numeroa), luokka (kirjain), 4 numeroa.
// Tunnus on VAIN työmuistissa: virran katketessa se katoaa tarkoituksella,
// jotta seuraavan ryhmän mittaukset eivät mene edellisen nimiin.
// Mittaukset toimivat ilman tunnusta, tunnus tarvitaan vain tietokantaan.
#define RYHMATUNNUS_PITUUS 7

extern char ryhmatunnus[RYHMATUNNUS_PITUUS + 1];   // "" = ei asetettu

bool ryhmaTunnusAsetettu();

// Kirjoittaa näytön tekstin "R=26A1234" tai "R=-------" (vähintään 10 tavua).
void ryhmaTunnusNaytolle(char* buf, size_t koko);
