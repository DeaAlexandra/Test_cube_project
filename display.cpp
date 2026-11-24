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

// ===== DIGITAALINEN: YLÄNÄYTTÖ =====
//   Digitaal.  R=-------
//   Sademäärä               (tyhjä, kun mittausta ei ole vielä valittu)
//
//   #=Valikko
// kanava: -1 = ei valittu, 0 = tuuli, 1 = sade, 2 = sisä, 3 = ulko.
void naytaDigitaaliYla(int kanava) {
  char tunnus[10];
  ryhmaTunnusNaytolle(tunnus, sizeof(tunnus));

  riviF(lcd1, 0, TXT("Digitaal.", "Digital"), tunnus);
  switch (kanava) {
    case 0:  riviF(lcd1, 1, TXT("Tuulen nopeus", "Wind speed"));                       break;
    case 1:  riviF(lcd1, 1, TXT("Sadem" LCD_AE LCD_AE "r" LCD_AE, "Rainfall"));        break;
    case 2:  riviF(lcd1, 1, TXT("Sis" LCD_AE "kosteus", "Indoor humidity"));          break;
    case 3:  riviF(lcd1, 1, TXT("Ulkokosteus", "Outdoor humidity"));                  break;
    default: riviF(lcd1, 1, F(""));                                                   break;
  }
  riviF(lcd1, 2, F(""));
  riviF(lcd1, 3, TXT("#=Valikko", "#=Menu"));
}

// ===== DIGITAALINEN: MITTAUKSEN VALINTA (alanäyttö) =====
//   Valitse mittaus
//   1=Tuuli   2=Sade
//   3=Sisä    4=Ulko
void naytaDigitaalinenValikko() {
  riviF(lcd2, 0, TXT("Valitse mittaus", "Select measurement"));
  riviF(lcd2, 1, TXT("1=Tuuli   2=Sade", "1=Wind    2=Rain"));
  riviF(lcd2, 2, TXT("3=Sis" LCD_AE "    4=Ulko", "3=Indoor  4=Outdoor"));
  riviF(lcd2, 3, F(""));
}

// ===== ANALOGINEN: YLÄNÄYTTÖ =====
//   Analoginen R=-------
//   Tuulen suunta           (tyhjä, kun mittausta ei ole vielä valittu)
//   Uout=Uin-1.2V           (tyhjä, kun mittausta ei ole vielä valittu)
//   #=Valikko
// sensori: 0 = ei valittu, 1 = lämpötila, 2 = valo, 3 = tuulen suunta.
// Kaavassa '*' tarkoittaa kertomerkkiä (näytössä ei ole ×-merkkiä).
void naytaAnalogYla(int sensori) {
  char tunnus[10];
  ryhmaTunnusNaytolle(tunnus, sizeof(tunnus));

  riviF(lcd1, 0, TXT("Analoginen", "Analog"), tunnus);
  switch (sensori) {
    case 1:
      riviF(lcd1, 1, TXT("L" LCD_AE "mp" LCD_OE "tila", "Temperature"));
      riviF(lcd1, 2, F("Uout=3*Uin"));
      break;
    case 2:
      riviF(lcd1, 1, TXT("Valo", "Light"));
      riviF(lcd1, 2, F("Uout=2*Uin"));
      break;
    case 3:
      riviF(lcd1, 1, TXT("Tuulen suunta", "Wind direction"));
      riviF(lcd1, 2, F("Uout=Uin-1.2V"));
      break;
    default:
      riviF(lcd1, 1, F(""));
      riviF(lcd1, 2, F(""));
      break;
  }
  riviF(lcd1, 3, TXT("#=Valikko", "#=Menu"));
}

// ===== ANALOGINEN: MITTAUKSEN VALINTA (alanäyttö) =====
//   Valitse mittaus
//   1=Lämpötila
//   2=Valo
//   3=Tuulen suunta
void naytaAnalogiaValikko() {
  riviF(lcd2, 0, TXT("Valitse mittaus", "Select measurement"));
  riviF(lcd2, 1, TXT("1=L" LCD_AE "mp" LCD_OE "tila", "1=Temperature"));
  riviF(lcd2, 2, TXT("2=Valo", "2=Light"));
  riviF(lcd2, 3, TXT("3=Tuulen suunta", "3=Wind direction"));
}

// ===== ANALOGINEN: TESTIPISTEET (alanäyttö) =====
//   1=1.700V  2=2.200V
//   3=2.700V  4=3.200V
//   5=3.900V
//   (rivi 4 varattu: 0=Tietokantamittaus tulee vaiheessa 7)
// Arvot ovat syöttöjännitteitä Uin (DAC), samat kuin analog.cpp:n taulukoissa.
static void testipiste(char* buf, size_t koko, uint8_t numero, int mV) {
  snprintf_P(buf, koko, PSTR("%u=%u.%03uV"), numero, mV / 1000, mV % 1000);
}

void naytaTestipisteValikko(int sensori) {
  const int* arvot = nullptr;
  if      (sensori == 1) arvot = LAMPOTILA_TESTIARVOT;
  else if (sensori == 2) arvot = VALO_TESTIARVOT;
  else if (sensori == 3) arvot = TUULISUUNTA_TESTIARVOT;
  if (!arvot) return;

  char vasen[11], oikea[11], rivi[21];

  testipiste(vasen, sizeof(vasen), 1, arvot[0]);
  testipiste(oikea, sizeof(oikea), 2, arvot[1]);
  snprintf_P(rivi, sizeof(rivi), PSTR("%-10s%s"), vasen, oikea);
  riviR(lcd2, 0, rivi);

  testipiste(vasen, sizeof(vasen), 3, arvot[2]);
  testipiste(oikea, sizeof(oikea), 4, arvot[3]);
  snprintf_P(rivi, sizeof(rivi), PSTR("%-10s%s"), vasen, oikea);
  riviR(lcd2, 1, rivi);

  testipiste(vasen, sizeof(vasen), 5, arvot[4]);
  riviR(lcd2, 2, vasen);

  riviF(lcd2, 3, TXT("0=Tietokantamittaus", "0=Database test"));
}

// ===== ANALOGINEN: TIETOKANTAMITTAUS (alanäyttö) =====
//   Ryhmätunnus puuttuu
//   Aseta tunnus
//   C-valikosta
// (* palaa testipisteisiin, # päävalikkoon)
void naytaRyhmatunnusPuuttuu() {
  riviF(lcd2, 0, TXT("Ryhm" LCD_AE "tunnus puuttuu", "Group ID missing"));
  riviF(lcd2, 1, TXT("Aseta tunnus", "Set group ID"));
  riviF(lcd2, 2, TXT("C-valikosta", "in C menu"));
  riviF(lcd2, 3, F(""));
}

//   Tietokantamittaus
//   Piste 2/5
//   Odota...
//   #=Keskeytä
void naytaTietokantamittausKaynnissa(uint8_t piste, uint8_t kaikki) {
  char rivi[21];
  riviF(lcd2, 0, TXT("Tietokantamittaus", "Database test"));
  snprintf_P(rivi, sizeof(rivi), PSTR("%S%u/%u"), TXT("Piste ", "Point "), piste, kaikki);
  riviR(lcd2, 1, rivi);
  riviF(lcd2, 2, TXT("Odota...", "Wait..."));
  riviF(lcd2, 3, TXT("#=Keskeyt" LCD_AE, "#=Abort"));
}

//   Hyväksytty!                 Virhe!
//   Pisteet: 5/5                Pisteet: 3/5
//   (vaihe 8: 0=Lähetä)         Virhe pisteessä: 2
//   *=Uusi     #=Valikko        *=Uusi     #=Valikko
// virhePiste = ensimmäinen hylätty piste (1-5), -1 jos kaikki hyväksytty.
void naytaTietokantaTulos(uint8_t hyvaksytyt, uint8_t kaikki, int8_t virhePiste) {
  char rivi[21];
  bool ok = (hyvaksytyt == kaikki);

  riviF(lcd2, 0, ok ? TXT("Hyv" LCD_AE "ksytty!", "Passed!")
                    : TXT("Virhe!", "Failed!"));

  snprintf_P(rivi, sizeof(rivi), PSTR("%S%u/%u"), TXT("Pisteet: ", "Points: "),
             hyvaksytyt, kaikki);
  riviR(lcd2, 1, rivi);

  if (ok) {
    riviF(lcd2, 2, F(""));
  } else {
    snprintf_P(rivi, sizeof(rivi), PSTR("%S%d"),
               TXT("Virhe pisteess" LCD_AE ": ", "Failed at point: "), virhePiste);
    riviR(lcd2, 2, rivi);
  }

  riviF(lcd2, 3, TXT("*=Uusi     #=Valikko", "*=Retry       #=Menu"));
}

// ===== DIGITAALINEN: MITTAUS JA TULOS (alanäyttö) =====

// Jännite muotoon "U=4.91V" (Arduinon snprintf ei tue liukulukuja).
static void janniteTekstiksi(char* buf, size_t koko, bool onArvo, uint16_t mV) {
  if (onArvo) snprintf_P(buf, koko, PSTR("U=%u.%02uV"), mV / 1000, (mV % 1000) / 10);
  else        snprintf_P(buf, koko, PSTR("U=--"));
}

// Rivi 1: "Hyväksytty!  U=4.91V" tai "Virhe!       U=4.91V"
static void digitaaliOtsikko(bool ok, bool onPulsseja, uint16_t jannite_mV) {
  char u[10];
  janniteTekstiksi(u, sizeof(u), onPulsseja, jannite_mV);
  riviF(lcd2, 0, ok ? TXT("Hyv" LCD_AE "ksytty!", "Passed!")
                    : TXT("Virhe!", "Failed!"), u);
}

//   Mittaus...
//   Odota 4s
//
//   #=Keskeytä
void naytaDigitaaliMittaus() {
  riviF(lcd2, 0, TXT("Mittaus...", "Measuring..."));
  riviF(lcd2, 1, TXT("Odota 4s", "Wait 4s"));
  riviF(lcd2, 2, F(""));
  riviF(lcd2, 3, TXT("#=Keskeyt" LCD_AE, "#=Abort"));
}

//   Hyväksytty!  U=4.91V
//   Lev:3.3ms
//                           (rivi 3 varattu: 0=Lähetä tulee vaiheessa 8)
//   *=Uusi     #=Valikko
// Hyväksyttyjen pulssien määrää ei näytetä: laite on validoinnin apuväline,
// ei mittalaite. Määrä tulostuu edelleen sarjamonitoriin (digital.cpp).
void naytaDigitaaliPulssiTulos(bool ok, uint32_t keskiLeveysUs, bool onPulsseja,
                               uint16_t hyvaksytyt, uint16_t kaikki, uint16_t jannite_mV) {
  (void)hyvaksytyt; (void)kaikki;      // ei näytöllä, ks. yllä
  char rivi[21];

  digitaaliOtsikko(ok, onPulsseja, jannite_mV);

  // Leveys millisekunteina yhdellä desimaalilla: kymmenesosat pyöristettynä.
  if (onPulsseja) {
    uint32_t kymmenykset = (keskiLeveysUs + 50UL) / 100UL;
    snprintf_P(rivi, sizeof(rivi), PSTR("%S%lu.%lums"), TXT("Lev:", "Width:"),
               kymmenykset / 10UL, kymmenykset % 10UL);
  } else {
    snprintf_P(rivi, sizeof(rivi), PSTR("%S--"), TXT("Lev:", "Width:"));
  }
  riviR(lcd2, 1, rivi);

  riviF(lcd2, 2, F(""));
  riviF(lcd2, 3, TXT("*=Uusi     #=Valikko", "*=Retry       #=Menu"));
}

//   Hyväksytty!  U=4.91V
//   6.9:OK 7.4:EI 7.9:OK
//                           (rivi 3 varattu: 0=Lähetä tulee vaiheessa 8)
//   *=Uusi     #=Valikko
void naytaDigitaaliKosteusTulos(bool ok, const bool taajuusOk[3], bool onPulsseja,
                                uint16_t jannite_mV) {
  char rivi[21];
  const __FlashStringHelper* ei = TXT("EI", "NO");
  const __FlashStringHelper* on = F("OK");

  digitaaliOtsikko(ok, onPulsseja, jannite_mV);

  snprintf_P(rivi, sizeof(rivi), PSTR("6.9:%S 7.4:%S 7.9:%S"),
             taajuusOk[0] ? on : ei,
             taajuusOk[1] ? on : ei,
             taajuusOk[2] ? on : ei);
  riviR(lcd2, 1, rivi);

  riviF(lcd2, 2, F(""));
  riviF(lcd2, 3, TXT("*=Uusi     #=Valikko", "*=Retry       #=Menu"));
}

// ===== ANALOGINEN TESTITULOS =====
//   Hyväksytty!             (tai Virhe!)
//   Mitattu:  1.523V
//   Odotettu: 1.500V
//   *=Uusi     #=Valikko
// Jännite muotoon "1.523V" (mV kokonaislukuna, ei liukulukuja).
static void janniteKolmellaDesimaalilla(char* buf, size_t koko, int mV) {
  if (mV < 0) mV = 0;
  snprintf_P(buf, koko, PSTR("%u.%03uV"), mV / 1000, mV % 1000);
}

void naytaAnalogiTulos(bool ok, int mitattuMV, int odotettuMV) {
  char arvo[10], rivi[21];

  riviF(lcd2, 0, ok ? TXT("Hyv" LCD_AE "ksytty!", "Passed!")
                    : TXT("Virhe!", "Failed!"));

  janniteKolmellaDesimaalilla(arvo, sizeof(arvo), mitattuMV);
  snprintf_P(rivi, sizeof(rivi), PSTR("%S%s"),
             TXT("Mitattu:  ", "Measured: "), arvo);
  riviR(lcd2, 1, rivi);

  janniteKolmellaDesimaalilla(arvo, sizeof(arvo), odotettuMV);
  snprintf_P(rivi, sizeof(rivi), PSTR("%S%s"),
             TXT("Odotettu: ", "Expected: "), arvo);
  riviR(lcd2, 2, rivi);

  riviF(lcd2, 3, TXT("*=Uusi     #=Valikko", "*=Retry       #=Menu"));
}

// ===== VIKATILA (vain alanäyttö) =====
//   !!!VIKATILA!!!
//   OIKOSULKU               (vian tyyppi, tai TESTI KESKEYTETTY)
//   Tarkista piiri
//   *=Uusi     #=Valikko
// Ylänäyttö ei muutu: siinä näkyy edelleen, mikä mittaus oli käynnissä.
void naytaVikatila(const __FlashStringHelper* viesti) {
  riviF(lcd2, 0, TXT("!!!VIKATILA!!!", "!!!FAULT!!!"));
  riviF(lcd2, 1, viesti);
  riviF(lcd2, 2, TXT("Tarkista piiri", "Check circuit"));
  riviF(lcd2, 3, TXT("*=Uusi     #=Valikko", "*=Retry       #=Menu"));
}