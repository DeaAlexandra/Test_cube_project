#pragma once
#include <Arduino.h>

// ===== VIKARAJAT =====
// ADC-ääriarvot - oikosulku tai avoin piiri
extern const int VIKA_ADC_MIN;
extern const int VIKA_ADC_MAX;

// Räikeä poikkeama - kerroin toleranssista
extern const float VIKA_TOLERANSSI_KERROIN;

// ===== TILA =====
extern bool vikatila_aktiivinen;

enum VikaTyyppi {
  VIKA_EI_VIKAA,
  VIKA_OIKOSULKU,      // ADC = 0
  VIKA_AVOIN_PIIRI,    // ADC = 1023
  VIKA_RAIKEA_POIKKEAMA,
  VIKA_EI_SIGNAALIA    // digitaalinen: piiri ei tuota pulsseja
};

// Tarkistaa ADC-arvon vikatilan varalta
VikaTyyppi tarkistaAdcVika(int adcArvo);

// Tarkistaa onko mitattu arvo räikeästi pois toleranssista (mV)
VikaTyyppi tarkistaPoikkeamaVika(int mitattuMV, int odotettuMV, int toleranssiMV);

// Näyttää vikailmoituksen molemmilla näytöillä ja sytyttää punaisen ledin.
// Ei pysäytä ohjelmaa: kutsuja keskeyttää mittauksen ja käyttäjä valitsee
// *=uusi mittaus tai #=päävalikko.
void kasitteleVikatila(VikaTyyppi vika);

// Nollaa vikatilan (kutsutaan resetin yhteydessä)
void vikatilaNollaa();