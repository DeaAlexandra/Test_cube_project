#pragma once
#include <Arduino.h>

// ===== TIETOKANTAAN LÄHETETTÄVÄT TULOKSET =====
// Tietokantamittauksen tulos odottaa tässä, kunnes ryhmä lähettää sen
// (vaihe 8: 0=Lähetä). Tulos on VAIN työmuistissa, kuten ryhmätunnuskin:
// virran katketessa se katoaa. Tulos tyhjennetään myös, kun ryhmätunnus
// poistetaan tai vaihdetaan, jotta se ei mene toisen ryhmän nimiin.

#define ANALOG_DB_PISTEET 5     // kaikki 5 testipistettä mitataan peräkkäin

struct AnalogTietokantaTulos {
  bool    valmis;                        // true = mittaus tehty, odottaa lähetystä
  int8_t  sensori;                       // 1 = lämpötila, 2 = valo, 3 = tuulen suunta
  uint8_t hyvaksytyt;                    // hyväksyttyjen pisteiden määrä
  int     syotto_mV[ANALOG_DB_PISTEET];  // Uin (DAC)
  int     odotettu_mV[ANALOG_DB_PISTEET];
  int     mitattu_mV[ANALOG_DB_PISTEET];
  bool    ok[ANALOG_DB_PISTEET];
};

extern AnalogTietokantaTulos analogDbTulos;

void tietokantaTyhjenna();   // poistaa odottavat tulokset
