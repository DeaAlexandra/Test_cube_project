#include "ui.h"
#include "analog.h"
#include "digital.h"
#include "display.h"
#include "errormode.h"
#include "lang.h"
#include <util/atomic.h>

// ===== NÄPPÄIMISTÖ =====
const byte RIVIT = 4;
const byte SARAKKEET = 4;
char napit[RIVIT][SARAKKEET] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rivi_pinnit[RIVIT]    = {26, 27, 28, 31};
byte sarake_pinnit[SARAKKEET] = {22, 23, 24, 25};
Keypad keypad = Keypad(makeKeymap(napit), rivi_pinnit, sarake_pinnit, RIVIT, SARAKKEET);

// ===== MULTIPLEXER (Arduino Mega 2560) =====
//   Signaali  Arduino-pinni  Portti  Bitti
//   EN        40             G       1   (PG1)  aktiivinen LOW
//   S0        41             G       0   (PG0)
//   S1        42             L       7   (PL7)
//   S2        43             L       6   (PL6)
#define MUX_EN  _BV(PG1)   // PORTG
#define MUX_S0  _BV(PG0)   // PORTG
#define MUX_S1  _BV(PL7)   // PORTL
#define MUX_S2  _BV(PL6)   // PORTL

// ===== TILAT =====
int  valittu_kanava  = -1;   // digitaalinen kanava 0-3 (tuuli, sade, sisä, ulko)

// ===== MULTIPLEXER =====
// Kanavanumeron (0-7) bitit ohjaavat valintalinjoja: bitti 0 -> S0,
// bitti 1 -> S1, bitti 2 -> S2. S1 ja S2 ovat samassa portissa, joten ne
// päivitetään yhdellä kirjoituksella.
//
// HUOM: Portissa G on myös ulkokosteuden pulssipinni PG2, jota Timer1:n
// keskeytys kääntää taustalla. Jos pääohjelma lukisi PORTG:n, keskeytys
// muuttaisi PG2:n ja pääohjelma kirjoittaisi vanhan arvon takaisin, pulssi
// katkeaisi. ATOMIC_BLOCK estää keskeytyksen tämän lyhyen hetken ajaksi.
// (Kääntäjä tekisi tästä todennäköisesti yhden atomisen sbi/cbi-käskyn joka
// tapauksessa, mutta suojaus tekee aikomuksen näkyväksi eikä riipu siitä.)
static void valitseKanava(int ch) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if (ch & 1) PORTG |=  MUX_S0;
    else        PORTG &= ~MUX_S0;
  }

  uint8_t l = PORTL & ~(MUX_S1 | MUX_S2);
  if (ch & 2) l |= MUX_S1;
  if (ch & 4) l |= MUX_S2;
  PORTL = l;
}

// ===== SETUP =====
void uiSetup() {
  DDRG |= MUX_EN | MUX_S0;     // lähdöiksi
  DDRL |= MUX_S1 | MUX_S2;
  PORTG &= ~MUX_EN;            // EN = LOW -> multiplekseri päällä
}

// ===== RESET =====
void uiResetoi() {
  digitalNollaa();
  vikatilaNollaa();
  valittu_kanava  = -1;
  Serial.println(txt("NOLLATTU", "RESET"));
  naytaTervetuloviesti();
}


// ===== NÄPPÄIMISTÖN KÄSITTELY =====
// Näppäimen merkitys riippuu tilasta. Kaikissa mittaustiloissa
// '#' palaa päävalikkoon.
void uiKasitteleNappi() {
  char nappi = keypad.getKey();
  if (!nappi) return;

  // ----- Analoginen mittaus -----
  if (analogSyottoAktiivinen()) {
    analogKasitteleValinta(nappi);
    return;
  }

  // ----- Digitaalinen mittaus -----
  switch (digitaali_tila) {
    case DIG_VALINTA:            // odotetaan kanavaa 1-4
      if (nappi >= '1' && nappi <= '4') {
        valittu_kanava = nappi - '1';
        int mux_kanava = (valittu_kanava == 0) ? 4 : valittu_kanava;
        valitseKanava(mux_kanava);                // ensin signaalitie kuntoon
        Serial.print(txt("Kanava: ", "Channel: "));
        Serial.println(nappi);
        naytaKanava(valittu_kanava);
        digitalAloitaMittaus(valittu_kanava);     // sitten mittaus käyntiin
      } else if (nappi == '#' || nappi == '*') {
        uiResetoi();
      }
      return;

    case DIG_MITTAA:             // mittaus käynnissä: vain keskeytys
      if (nappi == '#') uiResetoi();
      return;

    case DIG_TULOS:              // tulos tai vikailmoitus näytöllä
      if (nappi == '*') {
        vikatilaNollaa();
        naytaKanava(valittu_kanava);              // ylänäyttö takaisin (vika peitti sen)
        digitalAloitaMittaus(valittu_kanava);     // sama kanava uudelleen
      } else if (nappi == '#') {
        uiResetoi();
      }
      return;

    case DIG_POIS:
      break;                     // -> päävalikko
  }

  // ----- Päävalikko -----
  if (nappi == 'A') {
    analogAloitaValinta();
    naytaAnalogiaValikko();

  } else if (nappi == 'B') {
    kieli_englanti = !kieli_englanti;
    Serial.println(kieli_englanti ? "Language: English" : "Kieli: Suomi");
    naytaTervetuloviesti();

  } else if (nappi == 'D') {
    digitalAvaaValinta();
    Serial.println(txt("DIGITAALINEN VALINTA:", "DIGITAL SELECTION:"));
    Serial.println(txt("1=tuuli  2=sade  3=sisa  4=ulko",
                       "1=wind   2=rain  3=in    4=out"));
    naytaDigitaalinenValikko();

  } else if (nappi == '*') {
    uiResetoi();
  }
}