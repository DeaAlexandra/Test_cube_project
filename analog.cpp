#include "analog.h"
#include "led.h"
#include "errormode.h"
#include "display.h"
#include "lang.h"
#include "adc.h"

// ===== DAC-VAKIOARVOT (mV) =====
int DAC_LAMPOTILA_OUTA   = 1200;
int DAC_VALO_OUTA        = 1500;
int DAC_TUULISUUNTA_OUTA = 3200;
const int DAC_TUULISUUNTA_OUTB = 1200;

// ===== TOLERANSSIT (mV) =====
int TOL_LAMPOTILA   = 100;
int TOL_VALO        = 100;
int TOL_TUULISUUNTA = 100;

// ===== TESTIARVOT (mV) =====
const int LAMPOTILA_TESTIARVOT[]   = {500, 800, 1200, 1500, 1650};
const int VALO_TESTIARVOT[]        = {620, 900, 1200, 1800, 2250};
const int TUULISUUNTA_TESTIARVOT[] = {1700, 2200, 2700, 3200, 3900};
const int LAMPOTILA_MAARA   = 5;
const int VALO_MAARA        = 5;
const int TUULISUUNTA_MAARA = 5;

// ===== TILA =====
SyottoTila syotto_tila    = EI_SYOTTOA;
int        valittu_sensori = -1;

// ===== SETUP =====
void analogSetup() {
  MBED_UART.begin(9600);
  // Mittauspinni A1 asetetaan tuloksi adcSetup():ssa (adc.cpp).
  delay(2000);
  dacNollaa();
}

// ===== DAC =====
void dacSetBoth(int mvA, int mvB) {
  int arvoA = constrain(mvA, 0, 4095);
  int arvoB = constrain(mvB, 0, 4095);
  Serial.print("[DAC->Bob] AB");
  Serial.print(arvoA); Serial.print(","); Serial.println(arvoB);
  MBED_UART.print("AB");
  MBED_UART.print(arvoA); MBED_UART.print(",");
  MBED_UART.print(arvoB); MBED_UART.print("\n");
}

void dacNollaa() {
  Serial.println("[DAC->Bob] R");
  MBED_UART.print("R\n");
}

// ===== MITTAUS: 1s vakautus + keskiarvo 16 lukemasta =====
// Keskiarvo vähentää satunnaista kohinaa: 16 näytteen keskiarvossa kohina
// pienenee noin neljäsosaan (neliöjuuri 16:sta).
static int mittaaADC() {
  delay(1000);
  long summa = 0;
  for (int k = 0; k < 16; k++) {
    summa += adcLueKerran(ADC_KANAVA_MITTAUS);
    delay(10);
  }
  return (int)(summa / 16);
}

// ===== TESTI =====
static void suoritaTesti(int sensori, int pisteIndeksi) {
  const int* arvot;
  int   dacB;
  float kerroin;
  int   toleranssi;

  switch (sensori) {
    case 1:
      arvot      = LAMPOTILA_TESTIARVOT;
      dacB       = 0;
      kerroin    = 3.0;
      toleranssi = TOL_LAMPOTILA;
      Serial.println(txt("Lampotila (Vout=3*Vin):", "Temperature (Vout=3*Vin):"));
      break;
    case 2:
      arvot      = VALO_TESTIARVOT;
      dacB       = 0;
      kerroin    = 2.0;
      toleranssi = TOL_VALO;
      Serial.println(txt("Valo (Vout=2*Vin):", "Light (Vout=2*Vin):"));
      break;
    case 3:
      arvot      = TUULISUUNTA_TESTIARVOT;
      dacB       = DAC_TUULISUUNTA_OUTB;
      kerroin    = 1.0;
      toleranssi = TOL_TUULISUUNTA;
      Serial.println(txt("Tuulisuunta (Vout=Vin-1.2V):", "Wind dir (Vout=Vin-1.2V):"));
      break;
    default: return;
  }

  int vinMV      = arvot[pisteIndeksi];
  int odotettuMV = (int)(vinMV * kerroin) - dacB;

  dacSetBoth(vinMV, dacB);

  int adcArvo = mittaaADC();
  Serial.print("[ADC] "); Serial.println(adcArvo);

  VikaTyyppi vika = tarkistaAdcVika(adcArvo);
  if (vika != VIKA_EI_VIKAA) {
    dacNollaa();
    kasitteleVikatila(vika);         // ilmoitus + punainen ledi, ei pysäytä
    syotto_tila = NAYTTAA_TULOSTA;   // *=uusi testi, #=valikko
    return;
  }

  int  mitattuMV = (int)(adcArvo * (5000.0 / 1023.0));
  bool ok        = abs(mitattuMV - odotettuMV) <= toleranssi;

  Serial.print(txt("Mitattu: ", "Measured: "));
  Serial.print(mitattuMV / 1000.0, 3);
  Serial.print(txt("V  Odotettu: ", "V  Expected: "));
  Serial.print(odotettuMV / 1000.0, 3);
  Serial.print("V  (+-");
  Serial.print(toleranssi / 1000.0, 2);
  Serial.println("V)");
  Serial.println(ok ? "OK" : txt("VIRHE", "ERROR"));

  if (ok) {
    ledVihrea();
  } else {
    ledPunainen();
  }

  naytaAnalogiTulos(ok, mitattuMV, odotettuMV);
  syotto_tila = NAYTTAA_TULOSTA;
}

// ===== NAVIGOINTI =====
static void paluuPaavalikkoon() {
  dacNollaa();
  vikatilaNollaa();
  ledSammuta();
  syotto_tila     = EI_SYOTTOA;
  valittu_sensori = -1;
  naytaTervetuloviesti();
}

void analogAloitaValinta() {
  syotto_tila = ODOTTAA_ANALOGIKANAVA;
  Serial.println(txt("ANALOGINEN VALINTA:", "ANALOG SELECTION:"));
  Serial.println(txt("1=lampotila  2=valo  3=tuulisuunta",
                     "1=temp  2=light  3=wind dir"));
  Serial.println(txt("#=valikko", "#=menu"));
}

void analogKasitteleValinta(char nappi) {
  if (nappi == '#') { paluuPaavalikkoon(); return; }

  if (syotto_tila == NAYTTAA_TULOSTA) {
    if (nappi == '*') {
      // Vikailmoitus peitti myös ylänäytön -> palautetaan se ensin.
      if (vikatila_aktiivinen) {
        vikatilaNollaa();
        naytaTervetuloviesti();
      }
      syotto_tila = ODOTTAA_TESTIPISTETTA;
      naytaTestipisteValikko(valittu_sensori);
    }
    return;
  }

  if (syotto_tila == ODOTTAA_ANALOGIKANAVA && nappi >= '1' && nappi <= '3') {
    valittu_sensori = nappi - '0';
    syotto_tila = ODOTTAA_TESTIPISTETTA;
    naytaTestipisteValikko(valittu_sensori);
    return;
  }

  if (syotto_tila == ODOTTAA_TESTIPISTETTA && nappi >= '1' && nappi <= '5') {
    suoritaTesti(valittu_sensori, nappi - '1');
  }
}

bool analogSyottoAktiivinen() {
  return syotto_tila != EI_SYOTTOA;
}
