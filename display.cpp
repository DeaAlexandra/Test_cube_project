#include "display.h"
#include "analog.h"
#include "digital.h"
#include "lang.h"
#include "ryhma.h"

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

  naytaKaynnistys();
}

// ===== RIVIN KIRJOITUS =====
// Kirjoittaa koko 20 merkin rivin: 'vasen' (flash-teksti, TXT/F) alkuun ja
// 'oikea' (RAM-teksti, esim. "R=26A1234") oikeaan reunaan. Väli täytetään
// välilyönneillä, joten vanhaa tekstiä ei jää näkyviin eikä näyttöä tarvitse
// tyhjentää clear():lla (ei välkettä).
static void riviF(LiquidCrystal_I2C& lcd, uint8_t rivi,
                  const __FlashStringHelper* vasen, const char* oikea = "") {
  char buf[21];
  memset(buf, ' ', 20);
  buf[20] = '\0';
  PGM_P p = reinterpret_cast<PGM_P>(vasen);
  memcpy_P(buf, p, strnlen_P(p, 20));
  size_t lo = strlen(oikea); if (lo > 20) lo = 20;
  memcpy(buf + 20 - lo, oikea, lo);
  lcd.setCursor(0, rivi);
  lcd.print(buf);
}

// Sama RAM-tekstille (esim. syötetty tunnus), tasattuna vasemmalle.
static void riviR(LiquidCrystal_I2C& lcd, uint8_t rivi, const char* teksti) {
  char buf[21];
  memset(buf, ' ', 20);
  buf[20] = '\0';
  size_t l = strlen(teksti); if (l > 20) l = 20;
  memcpy(buf, teksti, l);
  lcd.setCursor(0, rivi);
  lcd.print(buf);
}

// ===== KÄYNNISTYSNÄKYMÄ =====
//   TEST CUBE
//
//   Käynnistyy...
//   Versio 2.0
// Näkyy setupin ajan (BOB:n käynnistysviive 2 s). Kieli on käynnistyksessä
// aina suomi, koska kielivalinta on vain työmuistissa.
void naytaKaynnistys() {
  riviF(lcd1, 0, F("TEST CUBE"));
  riviF(lcd1, 1, F(""));
  riviF(lcd1, 2, TXT("K" LCD_AE "ynnistyy...", "Starting..."));
  riviF(lcd1, 3, TXT("Versio " LAITE_VERSIO, "Version " LAITE_VERSIO));
  lcd2.clear();
}

// ===== PÄÄVALIKKO =====
//   A=Analog.  R=-------      (R=26A1234 kun ryhmätunnus on asetettu)
//   B=Asetukset
//   C=Ryhmävalikko
//   D=Digitaal.
// Napit aakkosjärjestyksessä, jotta oikean rivin löytää nopeasti.
// Alanäyttö tyhjä.
void naytaPaavalikko() {
  char tunnus[10];
  ryhmaTunnusNaytolle(tunnus, sizeof(tunnus));

  riviF(lcd1, 0, TXT("A=Analog.", "A=Analog"), tunnus);
  riviF(lcd1, 1, TXT("B=Asetukset", "B=Settings"));
  riviF(lcd1, 2, TXT("C=Ryhm" LCD_AE "valikko", "C=Group menu"));
  riviF(lcd1, 3, TXT("D=Digitaal.", "D=Digital"));
  lcd2.clear();
}

// ===== RYHMÄVALIKKO =====
//   Ryhmä      R=-------
//   1=Syötä tunnus
//   2=Poista tunnus
//              #=Valikko
// Ylänäyttö pysyy koko Ryhmävalikon ajan, alanäyttö vaihtuu.
void naytaRyhmavalikko() {
  char tunnus[10];
  ryhmaTunnusNaytolle(tunnus, sizeof(tunnus));

  riviF(lcd1, 0, TXT("Ryhm" LCD_AE, "Group"), tunnus);
  riviF(lcd1, 1, TXT("1=Sy" LCD_OE "t" LCD_AE " tunnus", "1=Enter ID"));
  riviF(lcd1, 2, TXT("2=Poista tunnus", "2=Delete ID"));
  riviF(lcd1, 3, TXT("           #=Valikko",     // oikeaan reunaan
                     "              #=Menu"));
  lcd2.clear();
}

//   Syötä ryhmätunnus
//   Vuosikurssi: 0-9      (vaihe: 2 numeroa, sitten luokka, sitten 4 numeroa)
//   26A12_
//   *=Poista merkki
// 'syotetty' = tähän asti syötetyt merkit, 'pituus' = niiden määrä.
void naytaTunnusSyotto(const char* syotetty, uint8_t pituus) {
  riviF(lcd2, 0, TXT("Sy" LCD_OE "t" LCD_AE " ryhm" LCD_AE "tunnus", "Enter group ID"));

  if (pituus < 2)       riviF(lcd2, 1, TXT("Vuosikurssi: 0-9", "Year: 0-9"));
  else if (pituus == 2) riviF(lcd2, 1, TXT("Luokka: A-D", "Class: A-D"));
  else                  riviF(lcd2, 1, TXT("Numero: 0-9", "Number: 0-9"));

  char buf[RYHMATUNNUS_PITUUS + 2];          // merkit + kursori '_' + loppumerkki
  memcpy(buf, syotetty, pituus);
  buf[pituus]     = '_';
  buf[pituus + 1] = '\0';
  riviR(lcd2, 2, buf);

  riviF(lcd2, 3, TXT("*=Poista merkki", "*=Delete char"));
}

//   Tunnus tallennettu
//   26A1234
void naytaTunnusTallennettu() {
  riviF(lcd2, 0, TXT("Tunnus tallennettu", "ID saved"));
  riviR(lcd2, 1, ryhmatunnus);
  riviF(lcd2, 2, F(""));
  riviF(lcd2, 3, F(""));
}

//   Poista tunnus
//   26A1234
//
//   *=Kyllä #=Ei
void naytaTunnusPoistoVahvistus() {
  riviF(lcd2, 0, TXT("Poista tunnus", "Delete group ID"));
  riviR(lcd2, 1, ryhmatunnus);
  riviF(lcd2, 2, F(""));
  riviF(lcd2, 3, TXT("*=Kyll" LCD_AE " #=Ei", "*=Yes   #=No"));
}

//   Tunnus poistettu!
void naytaTunnusPoistettu() {
  riviF(lcd2, 0, TXT("Tunnus poistettu!", "Group ID deleted!"));
  riviF(lcd2, 1, F(""));
  riviF(lcd2, 2, F(""));
  riviF(lcd2, 3, F(""));
}

//   Ei ryhmätunnusta      (2=Poista, kun tunnusta ei ole)
void naytaEiTunnusta() {
  riviF(lcd2, 0, TXT("Ei ryhm" LCD_AE "tunnusta", "No group ID"));
  riviF(lcd2, 1, F(""));
  riviF(lcd2, 2, F(""));
  riviF(lcd2, 3, F(""));
}

// ===== ASETUKSET =====
//   Asetukset  R=-------
//   1=Laitteen tiedot
//   2=Kieli
//   3=Verkko   #=Valikko
// Ylänäyttö pysyy tällaisena koko Asetukset-valikon ajan, alanäyttö vaihtuu
// valinnan mukaan.
void naytaAsetukset() {
  char tunnus[10];
  ryhmaTunnusNaytolle(tunnus, sizeof(tunnus));

  riviF(lcd1, 0, TXT("Asetukset", "Settings"), tunnus);
  riviF(lcd1, 1, TXT("1=Laitteen tiedot", "1=Device info"));
  riviF(lcd1, 2, TXT("2=Kieli", "2=Language"));
  riviF(lcd1, 3, TXT("3=Verkko   #=Valikko", "3=Network  #=Menu"));
  lcd2.clear();
}

//   Versio 2.0
//   Päivitetty 24.11.26
void naytaLaitteenTiedot() {
  riviF(lcd2, 0, TXT("Versio " LAITE_VERSIO, "Version " LAITE_VERSIO));
  riviF(lcd2, 1, TXT("P" LCD_AE "ivitetty " LAITE_PAIVITETTY,
                     "Updated " LAITE_PAIVITETTY));
  riviF(lcd2, 2, F(""));
  riviF(lcd2, 3, F(""));
}

//   1=Suomi
//   2=English
// Kielten nimet kirjoitetaan aina omalla kielellään, jotta oikean löytää
// vaikka nykyistä kieltä ei ymmärtäisi.
void naytaKielivalinta() {
  riviF(lcd2, 0, F("1=Suomi"));
  riviF(lcd2, 1, F("2=English"));
  riviF(lcd2, 2, F(""));
  riviF(lcd2, 3, F(""));
}

//   Yhdistetty            tai   Ei yhteyttä
//                                (tyhjä)
//                                Katso ohjekirja
void naytaVerkonTila(bool yhteys) {
  if (yhteys) {
    riviF(lcd2, 0, TXT("Yhdistetty", "Connected"));
    riviF(lcd2, 2, F(""));
  } else {
    riviF(lcd2, 0, TXT("Ei yhteytt" LCD_AE, "No connection"));
    riviF(lcd2, 2, TXT("Katso ohjekirja", "See user manual"));
  }
  riviF(lcd2, 1, F(""));
  riviF(lcd2, 3, F(""));
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