#include "errormode.h"
#include "led.h"
#include "display.h"
#include "lang.h"

// ===== VIKARAJAT =====
const int VIKA_ADC_MIN = 3;      // ADC <= 3  → oikosulku  (~15 mV)
const int VIKA_ADC_MAX = 1020;   // ADC >= 1020 → avoin piiri (~4.99 V)
const float VIKA_TOLERANSSI_KERROIN = 5.0;  // Yli 5x toleranssi = vika

// ===== TILA =====
bool vikatila_aktiivinen = false;

// ===== TARKISTUKSET =====
VikaTyyppi tarkistaAdcVika(int adcArvo) {
  if (adcArvo <= VIKA_ADC_MIN) {
    return VIKA_OIKOSULKU;
  }
  if (adcArvo >= VIKA_ADC_MAX) {
    return VIKA_AVOIN_PIIRI;
  }
  return VIKA_EI_VIKAA;
}

VikaTyyppi tarkistaPoikkeamaVika(int mitattuMV, int odotettuMV, int toleranssiMV) {
  int poikkeama = abs(mitattuMV - odotettuMV);
  if (poikkeama > (toleranssiMV * VIKA_TOLERANSSI_KERROIN)) {
    return VIKA_RAIKEA_POIKKEAMA;
  }
  return VIKA_EI_VIKAA;
}

// ===== VIKATILAN KÄSITTELY =====
void kasitteleVikatila(VikaTyyppi vika) {
  if (vika == VIKA_EI_VIKAA) return;

  vikatila_aktiivinen = true;

  const char* viesti;
  switch (vika) {
    case VIKA_OIKOSULKU:
      viesti = txt("OIKOSULKU", "SHORT CIRCUIT");
      break;
    case VIKA_AVOIN_PIIRI:
      viesti = txt("AVOIN PIIRI", "OPEN CIRCUIT");
      break;
    case VIKA_RAIKEA_POIKKEAMA:
      viesti = txt("POIKKEAMA", "DEVIATION");
      break;
    case VIKA_EI_SIGNAALIA:
      viesti = txt("EI SIGNAALIA", "NO SIGNAL");
      break;
    default:
      viesti = txt("TUNTEMATON", "UNKNOWN");
      break;
  }

  // Ilmoitus ja punainen ledi. Aiemmin tässä oli silmukka, joka vilkutti
  // lediä ja odotti *-näppäintä: koko ohjelma jumittui siihen asti.
  // Nyt funktio palaa heti, ja kutsuja siirtyy tulostilaan, jossa
  // näppäimet käsitellään normaalisti pääsilmukassa.
  naytaVikatila(viesti);
  ledPunainen();
}

void vikatilaNollaa() {
  vikatila_aktiivinen = false;
  ledSammuta();
}