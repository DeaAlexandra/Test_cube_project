#include "tietokanta.h"

AnalogTietokantaTulos analogDbTulos;   // globaali -> nollautuu käynnistyksessä

void tietokantaTyhjenna() {
  memset(&analogDbTulos, 0, sizeof(analogDbTulos));
}
