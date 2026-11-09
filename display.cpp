#include "display.h"
#include "analog.h"
#include "digital.h"
#include "lang.h"

// ===== NÄYTÖT =====
LiquidCrystal_I2C lcd1(0x23, 20, 4);
LiquidCrystal_I2C lcd2(0x27, 20, 4);

// ===== ERIKOISMERKIT =====
// 0 = ä, 1 = ö, 2 = Ä, 3 = Ö
byte merkkiA[8] = {
  0b01010,
  0b00000,
  0b01110,
  0b00001,
  0b01111,
  0b10001,
  0b01111,
  0b00000
};

byte merkkiO[8] = {
  0b01010,
  0b00000,
  0b01110,
  0b10001,
  0b10001,
  0b10001,
  0b01110,
  0b00000
};

byte merkkiIsoA[8] = {
  0b01010,
  0b00100,
  0b01010,
  0b10001,
  0b11111,
  0b10001,
  0b10001,
  0b00000
};

byte merkkiIsoO[8] = {
  0b01010,
  0b01110,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b01110,
  0b00000
};

// ===== SETUP =====
void displaySetup() {
  lcd1.init();
  lcd1.backlight();
  lcd2.init();
  lcd2.backlight();

  lcd1.createChar(0, merkkiA);
  lcd1.createChar(1, merkkiO);
  lcd1.createChar(2, merkkiIsoA);
  lcd1.createChar(3, merkkiIsoO);

  lcd2.createChar(0, merkkiA);
  lcd2.createChar(1, merkkiO);
  lcd2.createChar(2, merkkiIsoA);
  lcd2.createChar(3, merkkiIsoO);

  naytaTervetuloviesti();
}

// ===== YLÄNÄYTTÖ (pysyvä otsikko + tila) =====
void naytaTervetuloviesti() {
  lcd1.clear();
  lcd2.clear();
  lcd1.setCursor(0, 0);
  if (kieli_englanti) {
    lcd1.print("WEATHER STN TEST");
  } else {
    lcd1.print("S");
    lcd1.write(2);          // Ä
    lcd1.write(2);          // Ä
    lcd1.print("ASEMA TESTAUS");
  }
  lcd1.setCursor(0, 1);
  lcd1.print(txt("Valmis", "Ready"));
  lcd1.setCursor(0, 2);
  lcd1.print(txt("A=analoginen", "A=analog"));
  lcd1.setCursor(0, 3);
  lcd1.print(txt("D=digitaalinen", "D=digital"));
}

void naytaKanava(int kanava) {
  lcd1.clear();
  lcd1.setCursor(0, 0);
  if (kieli_englanti) {
    lcd1.print("WEATHER STN TEST");
  } else {
    lcd1.print("S");
    lcd1.write(2);          // Ä
    lcd1.write(2);          // Ä
    lcd1.print("ASEMA TESTAUS");
  }

  lcd1.setCursor(0, 1);
  lcd1.print(txt("DIGITAALINEN:", "DIGITAL:"));

  lcd1.setCursor(0, 2);
  if (kieli_englanti) {
    switch (kanava) {
      case 0: lcd1.print("Wind speed");    break;
      case 1: lcd1.print("Rain");          break;
      case 2: lcd1.print("Indoor humidity"); break;
      case 3: lcd1.print("Outdoor humidity"); break;
    }
  } else {
    switch (kanava) {
      case 0: lcd1.print("Tuuli"); break;
      case 1: lcd1.print("Sade");  break;
      case 2:
        lcd1.print("Sis");
        lcd1.write(0);       // ä
        lcd1.print("kosteus");
        break;
      case 3: lcd1.print("Ulkokosteus"); break;
    }
  }

  lcd1.setCursor(0, 3);
  lcd1.print(txt("#=valikko", "#=menu"));
}

// ===== ALANÄYTTÖ - VALIKOT =====
void naytaDigitaalinenValikko() {
  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print(txt("DIGITAALINEN:", "DIGITAL:"));
  if (kieli_englanti) {
    lcd2.setCursor(0, 1);
    lcd2.print("1=wind  2=rain");
    lcd2.setCursor(0, 2);
    lcd2.print("3=in    4=out");
  } else {
    lcd2.setCursor(0, 1);
    lcd2.print("1=tuuli  2=sade");
    lcd2.setCursor(0, 2);
    lcd2.print("3=sis");
    lcd2.write(0);          // ä
    lcd2.print("   4=ulko");
  }
}

void naytaAnalogiaValikko() {
  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print(txt("ANALOGINEN:", "ANALOG:"));
  if (kieli_englanti) {
    lcd2.setCursor(0, 1);
    lcd2.print("1=temperature");
    lcd2.setCursor(0, 2);
    lcd2.print("2=light");
    lcd2.setCursor(0, 3);
    lcd2.print("3=wind direction");
  } else {
    lcd2.setCursor(0, 1);
    lcd2.print("1=l");
    lcd2.write(0);          // ä
    lcd2.print("mp");
    lcd2.write(1);          // ö
    lcd2.print("tila");
    lcd2.setCursor(0, 2);
    lcd2.print("2=valo");
    lcd2.setCursor(0, 3);
    lcd2.print("3=tuulen suunta");
  }
}

// ===== TESTIPISTE-VALIKKO =====
void naytaTestipisteValikko(int sensori) {
  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print(txt("Valitse arvo:", "Select value:"));

  const int* arvot = nullptr;
  if      (sensori == 1) arvot = LAMPOTILA_TESTIARVOT;
  else if (sensori == 2) arvot = VALO_TESTIARVOT;
  else if (sensori == 3) arvot = TUULISUUNTA_TESTIARVOT;
  if (!arvot) return;

  // Rivi 1: pisteet 1 ja 2
  // Rivi 2: pisteet 3 ja 4
  // Rivi 3: piste 5
  char buf[21];
  snprintf(buf, sizeof(buf), "1.%dmV 2.%dmV", arvot[0], arvot[1]);
  lcd2.setCursor(0, 1);
  lcd2.print(buf);
  snprintf(buf, sizeof(buf), "3.%dmV 4.%dmV", arvot[2], arvot[3]);
  lcd2.setCursor(0, 2);
  lcd2.print(buf);
  snprintf(buf, sizeof(buf), "5.%dmV", arvot[4]);
  lcd2.setCursor(0, 3);
  lcd2.print(buf);
}

// ===== ALANÄYTTÖ - DIGITAALINEN MITTAUS =====

// Kirjoittaa 20 merkin rivin: 'vasen' alkuun ja 'oikea' oikeaan reunaan,
// väli täytetään välilyönneillä. Koko rivi kirjoitetaan kerralla, joten
// vanhaa tekstiä ei jää näkyviin.
static void kirjoitaRivi(LiquidCrystal_I2C& lcd, uint8_t rivi,
                         const char* vasen, const char* oikea) {
  char buf[21];
  memset(buf, ' ', 20);
  buf[20] = '\0';
  size_t lv = strlen(vasen); if (lv > 20) lv = 20;
  size_t lo = strlen(oikea); if (lo > 20) lo = 20;
  memcpy(buf, vasen, lv);
  memcpy(buf + 20 - lo, oikea, lo);
  lcd.setCursor(0, rivi);
  lcd.print(buf);
}

// Jännite muotoon "U=4.91V" (Arduinon snprintf ei tue liukulukuja).
static void janniteTekstiksi(char* buf, size_t koko, bool onArvo, uint16_t mV) {
  if (onArvo) snprintf(buf, koko, "U=%u.%02uV", mV / 1000, (mV % 1000) / 10);
  else        snprintf(buf, koko, "U=--");
}

static const char* tulosTeksti(bool ok) {
  return ok ? "OK!" : txt("VIRHE!", "ERROR!");
}

void naytaDigitaaliMittaus() {
  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print(txt("Mittaus...", "Measuring..."));
  lcd2.setCursor(0, 1);
  lcd2.print(txt("Odota 4 s", "Wait 4 s"));
  lcd2.setCursor(0, 3);
  lcd2.print(txt("#=keskeyta", "#=abort"));
}

//   TULOS
//   Lev: 3.3ms      8/10
//   U=4.91V          OK!
//   *=uusi  #=valikko
void naytaDigitaaliPulssiTulos(bool ok, uint32_t keskiLeveysUs, bool onPulsseja,
                               uint16_t hyvaksytyt, uint16_t kaikki, uint16_t jannite_mV) {
  char vasen[21], oikea[21];

  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print(txt("TULOS", "RESULT"));

  // Leveys millisekunteina yhdellä desimaalilla: kymmenesosat pyöristettynä.
  if (onPulsseja) {
    uint32_t kymmenykset = (keskiLeveysUs + 50UL) / 100UL;
    snprintf(vasen, sizeof(vasen), "%s%lu.%lums", txt("Lev: ", "Width: "),
             kymmenykset / 10UL, kymmenykset % 10UL);
  } else {
    snprintf(vasen, sizeof(vasen), "%s--", txt("Lev: ", "Width: "));
  }
  snprintf(oikea, sizeof(oikea), "%u/%u", hyvaksytyt, kaikki);
  kirjoitaRivi(lcd2, 1, vasen, oikea);

  janniteTekstiksi(vasen, sizeof(vasen), onPulsseja, jannite_mV);
  kirjoitaRivi(lcd2, 2, vasen, tulosTeksti(ok));

  lcd2.setCursor(0, 3);
  lcd2.print(txt("*=uusi  #=valikko", "*=retry #=menu"));
}

//   TULOS
//   6.9:OK 7.4:XX 7.9:OK
//   U=4.91V       VIRHE!
//   *=uusi  #=valikko
void naytaDigitaaliKosteusTulos(bool ok, const bool taajuusOk[3], bool onPulsseja,
                                uint16_t jannite_mV) {
  char rivi[21];

  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print(txt("TULOS", "RESULT"));

  snprintf(rivi, sizeof(rivi), "6.9:%s 7.4:%s 7.9:%s",
           taajuusOk[0] ? "OK" : "XX",
           taajuusOk[1] ? "OK" : "XX",
           taajuusOk[2] ? "OK" : "XX");
  lcd2.setCursor(0, 1);
  lcd2.print(rivi);

  janniteTekstiksi(rivi, sizeof(rivi), onPulsseja, jannite_mV);
  kirjoitaRivi(lcd2, 2, rivi, tulosTeksti(ok));

  lcd2.setCursor(0, 3);
  lcd2.print(txt("*=uusi  #=valikko", "*=retry #=menu"));
}

// ===== ANALOGINEN TESTITULOS =====
void naytaAnalogiTulos(bool ok, int mitattuMV, int odotettuMV) {
  lcd2.clear();
  delay(5);
  lcd2.setCursor(0, 0);
  lcd2.print(ok ? txt("OK!", "OK!") : txt("VIRHE!", "ERROR!"));
  lcd2.setCursor(0, 1);
  lcd2.print(txt("Mitattu:  ", "Measured: "));
  lcd2.print(mitattuMV / 1000.0, 3);
  lcd2.print("V");
  lcd2.setCursor(0, 2);
  lcd2.print(txt("Odotettu: ", "Expected: "));
  lcd2.print(odotettuMV / 1000.0, 3);
  lcd2.print("V");
  lcd2.setCursor(0, 3);
  lcd2.print(txt("*=uusi  #=valikko", "*=retry #=menu"));
}

// ===== VIKATILA =====
void naytaVikatila(const char* viesti) {
  lcd1.clear();
  lcd1.setCursor(0, 0);
  lcd1.print(txt("!!! VIKATILA !!!", "!!! FAULT !!!"));
  lcd1.setCursor(0, 1);
  lcd1.print(viesti);
  lcd1.setCursor(0, 2);
  lcd1.print(txt("Tarkista piiri", "Check circuit"));
  lcd1.setCursor(0, 3);
  lcd1.print(txt("*=uusi  #=valikko", "*=retry #=menu"));

  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print(txt("!!! VIKATILA !!!", "!!! FAULT !!!"));
  lcd2.setCursor(0, 1);
  lcd2.print(viesti);
  lcd2.setCursor(0, 2);
  lcd2.print(txt("Testi keskeytetty", "Test aborted"));
  lcd2.setCursor(0, 3);
  lcd2.print(txt("*=uusi  #=valikko", "*=retry #=menu"));
}