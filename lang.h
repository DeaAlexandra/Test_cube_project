#pragma once
#include <Arduino.h>

extern bool kieli_englanti;

// Palauttaa fi tai en sen mukaan, kumpi kieli on valittuna.
// HUOM: tekstit ovat työmuistissa (RAM). Uusissa näkymissä käytetään TXT().
inline const char* txt(const char* fi, const char* en) {
  return kieli_englanti ? en : fi;
}

// ===== TEKSTIT OHJELMAMUISTISTA (FLASH) =====
// TXT("suomeksi", "in English") toimii kuten txt(), mutta molemmat tekstit
// jäävät ohjelmamuistiin eivätkä vie työmuistia. RAM:ia on vain 8 kt ja
// jokainen tavallinen tekstivakio kopioidaan sinne käynnistyksessä, joten
// kaksikielisten näkymien tekstit kannattaa pitää flashissa.
// Käy suoraan: lcd.print(TXT(...)) ja Serial.println(TXT(...)).
#define TXT(fi, en) (kieli_englanti ? F(en) : F(fi))

// ===== SKANDIT LCD-TEKSTEIHIN =====
// Omat merkit ovat näytön CGRAM-paikoissa 0-3 (display.cpp). Samat merkit
// löytyvät myös koodeista 8-11, ja niitä voi käyttää tekstin sisällä
// (koodi 0 päättäisi C-merkkijonon). Liitetään tekstiin erillisinä paloina:
//   "Ryhm" LCD_AE "valikko"  ->  Ryhmävalikko
// Vain LCD:lle, sarjamonitorissa nämä näkyvät ohjausmerkkeinä.
#define LCD_AE      "\x08"   // ä
#define LCD_OE      "\x09"   // ö
#define LCD_AE_ISO  "\x0A"   // Ä
#define LCD_OE_ISO  "\x0B"   // Ö
