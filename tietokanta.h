#pragma once
#include <Arduino.h>

// ===== TIETOKANTAAN LÄHETETTÄVÄT TULOKSET =====
// Tietokantamittauksen tulos odottaa tässä, kunnes ryhmä lähettää sen
// napilla 0 (0=Lähetä). Odottamassa on aina enintään yksi tulos: uusi
// mittaus korvaa edellisen. Tulos on VAIN työmuistissa, kuten
// ryhmätunnuskin: virran katketessa se katoaa. Tulos tyhjennetään myös,
// kun ryhmätunnus poistetaan tai vaihdetaan, jotta se ei mene toisen
// ryhmän nimiin, ja kun se on lähetetty onnistuneesti.

#define ANALOG_DB_PISTEET 5     // kaikki 5 testipistettä mitataan peräkkäin

// Mikä tulos odottaa lähetystä
enum DbTyyppi : uint8_t { DB_EI, DB_ANALOG, DB_DIGI };
extern DbTyyppi db_odottaa;

// ----- Analoginen: 5 pisteen sarja (nappi 0 testipistevalikossa) -----
struct AnalogTietokantaTulos {
  int8_t  sensori;                       // 1 = lämpötila, 2 = valo, 3 = tuulen suunta
  uint8_t hyvaksytyt;                    // hyväksyttyjen pisteiden määrä
  int     syotto_mV[ANALOG_DB_PISTEET];  // Uin (DAC)
  int     odotettu_mV[ANALOG_DB_PISTEET];
  int     mitattu_mV[ANALOG_DB_PISTEET];
  bool    ok[ANALOG_DB_PISTEET];
};
extern AnalogTietokantaTulos analogDbTulos;

// ----- Digitaalinen: tavallinen mittaus, kun ryhmätunnus on asetettu -----
struct DigiTietokantaTulos {
  int8_t   kanava;          // 0 = tuuli, 1 = sade, 2 = sisä, 3 = ulko
  bool     ok;
  uint16_t jannite_mV;      // keskimääräinen HIGH-taso
  // tuuli ja sade
  uint32_t leveys_us;       // keskimääräinen pulssin leveys
  uint16_t hyvaksytyt;
  uint16_t kaikki;
  // kosteus (6.9 / 7.4 / 7.9 kHz)
  uint16_t taajuus_hz[3];   // mitattu lähdön taajuus
  bool     taajuusOk[3];
};
extern DigiTietokantaTulos digiDbTulos;

void tietokantaTyhjenna();      // poistaa odottavan tuloksen
bool tietokantaVoiLahettaa();   // odottaako HYVÄKSYTTY tulos (näytetään 0=Lähetä)

// Kirjoittaa odottavan tuloksen JSON-muodossa annettuun virtaan
// (Serial nyt, myöhemmin MQTT-asiakas). Ks. muoto tietokanta.cpp.
void tietokantaKirjoitaJson(Print& out);

// 0=Lähetä: Lähetetään... -> Lähetetty! / Lähetys epäonnistui.
// Onnistuneen lähetyksen jälkeen tulos poistetaan, jotta sitä ei lähetetä
// kahdesti. Epäonnistunut tulos jää odottamaan (0=Yritä uudelleen).
void tietokantaLaheta();
