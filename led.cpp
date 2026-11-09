#include "led.h"

// ===== PINNIT JA PORTTIBITIT (Arduino Mega 2560) =====
// Arduinon pinninumero on vain "nimi". Todellisuudessa jokainen pinni on
// jonkin 8-bittisen portin (A, B, C ... L) yksi bitti.
//
//   Väri      Arduino-pinni   Portti  Bitti
//   Punainen  3               E       5   (PE5)
//   Vihreä    5               E       3   (PE3)
//   Sininen   6               H       3   (PH3)
//
// _BV(n) = (1 << n) eli bittimaski, jossa vain bitti n on ykkönen.
#define LED_R  _BV(PE5)   // portti E
#define LED_G  _BV(PE3)   // portti E
#define LED_B  _BV(PH3)   // portti H

// ===== SETUP =====
// DDRx (Data Direction Register): bitti 1 = lähtö, bitti 0 = tulo.
// |= asettaa vain valitut bitit ykköseksi ja jättää portin muut pinnit ennalleen.
void ledSetup() {
  DDRE |= LED_R | LED_G;
  DDRH |= LED_B;
  ledSammuta();
}

// ===== VÄRIN ASETUS =====
// PORTx: lähtöpinnin tila, bitti 1 = HIGH (5 V), bitti 0 = LOW (0 V).
// Punainen ja vihreä ovat samassa portissa, joten ne päivitetään yhdellä
// kirjoituksella: luetaan PORTE, nollataan ledibitit, asetetaan halutut ja
// kirjoitetaan takaisin. Värit vaihtuvat samalla kellojaksolla, eikä välissä
// näy hetkellisesti väärää väriä.
static void asetaVari(bool r, bool g, bool b) {
  uint8_t e = PORTE & ~(LED_R | LED_G);
  if (r) e |= LED_R;
  if (g) e |= LED_G;
  PORTE = e;

  if (b) PORTH |=  LED_B;
  else   PORTH &= ~LED_B;
}

// ===== VÄRIT =====
void ledSammuta()   { asetaVari(false, false, false); }
void ledVihrea()    { asetaVari(false, true,  false); }
void ledKeltainen() { asetaVari(true,  true,  false); }
void ledPunainen()  { asetaVari(true,  false, false); }
