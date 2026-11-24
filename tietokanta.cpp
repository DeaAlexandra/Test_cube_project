#include "tietokanta.h"
#include "display.h"
#include "lang.h"
#include "ryhma.h"
#include "verkko.h"

// Globaalit -> nollautuvat käynnistyksessä
DbTyyppi              db_odottaa = DB_EI;
AnalogTietokantaTulos analogDbTulos;
DigiTietokantaTulos   digiDbTulos;

void tietokantaTyhjenna() {
  db_odottaa = DB_EI;
  memset(&analogDbTulos, 0, sizeof(analogDbTulos));
  memset(&digiDbTulos,   0, sizeof(digiDbTulos));
}

bool tietokantaVoiLahettaa() {
  if (!ryhmaTunnusAsetettu()) return false;
  if (db_odottaa == DB_ANALOG) return analogDbTulos.hyvaksytyt == ANALOG_DB_PISTEET;
  if (db_odottaa == DB_DIGI)   return digiDbTulos.ok;
  return false;
}

// ===== JSON =====
// Kentät ovat ASCII-merkkejä (ei skandeja), jotta palvelin voi tarkistaa ne
// tarkasti. Aikaleimaa ei ole, koska laitteessa ei ole kelloa: palvelin
// merkitsee vastaanottoajan. Esimerkit:
//
// {"laite":"TESTCUBE","versio":"2.0","ryhma":"26A1234","tyyppi":"analog",
//  "mittaus":"lampotila","ok":true,
//  "pisteet":[{"uin":500,"odotettu":1500,"mitattu":1510,"ok":true},...]}
//
// {"laite":"TESTCUBE","versio":"2.0","ryhma":"26A1234","tyyppi":"digital",
//  "mittaus":"sade","ok":true,"u_mv":4910,
//  "leveys_us":3300,"hyvaksytyt":5,"kaikki":5}
//
// {"laite":"TESTCUBE","versio":"2.0","ryhma":"26A1234","tyyppi":"digital",
//  "mittaus":"sisakosteus","ok":true,"u_mv":4910,
//  "taajuudet":[{"hz":6900,"ok":true},{"hz":7400,"ok":true},{"hz":7900,"ok":true}]}

static void jsonBool(Print& out, bool b) {
  out.print(b ? F("true") : F("false"));
}

static void jsonAlku(Print& out, const __FlashStringHelper* tyyppi,
                     const __FlashStringHelper* mittaus, bool ok) {
  out.print(F("{\"laite\":\"TESTCUBE\",\"versio\":\"" LAITE_VERSIO "\",\"ryhma\":\""));
  out.print(ryhmatunnus);
  out.print(F("\",\"tyyppi\":\""));   out.print(tyyppi);
  out.print(F("\",\"mittaus\":\""));  out.print(mittaus);
  out.print(F("\",\"ok\":"));         jsonBool(out, ok);
}

static const __FlashStringHelper* analogNimi(int8_t sensori) {
  switch (sensori) {
    case 1:  return F("lampotila");
    case 2:  return F("valo");
    case 3:  return F("tuulensuunta");
    default: return F("tuntematon");
  }
}

static const __FlashStringHelper* digiNimi(int8_t kanava) {
  switch (kanava) {
    case 0:  return F("tuuli");
    case 1:  return F("sade");
    case 2:  return F("sisakosteus");
    case 3:  return F("ulkokosteus");
    default: return F("tuntematon");
  }
}

void tietokantaKirjoitaJson(Print& out) {
  if (db_odottaa == DB_ANALOG) {
    const AnalogTietokantaTulos& a = analogDbTulos;
    jsonAlku(out, F("analog"), analogNimi(a.sensori),
             a.hyvaksytyt == ANALOG_DB_PISTEET);
    out.print(F(",\"pisteet\":["));
    for (uint8_t i = 0; i < ANALOG_DB_PISTEET; i++) {
      if (i) out.print(',');
      out.print(F("{\"uin\":"));       out.print(a.syotto_mV[i]);
      out.print(F(",\"odotettu\":"));  out.print(a.odotettu_mV[i]);
      out.print(F(",\"mitattu\":"));   out.print(a.mitattu_mV[i]);
      out.print(F(",\"ok\":"));        jsonBool(out, a.ok[i]);
      out.print('}');
    }
    out.print(F("]}"));

  } else if (db_odottaa == DB_DIGI) {
    const DigiTietokantaTulos& d = digiDbTulos;
    jsonAlku(out, F("digital"), digiNimi(d.kanava), d.ok);
    out.print(F(",\"u_mv\":")); out.print(d.jannite_mV);
    if (d.kanava <= 1) {
      out.print(F(",\"leveys_us\":"));  out.print(d.leveys_us);
      out.print(F(",\"hyvaksytyt\":")); out.print(d.hyvaksytyt);
      out.print(F(",\"kaikki\":"));     out.print(d.kaikki);
    } else {
      out.print(F(",\"taajuudet\":["));
      for (uint8_t i = 0; i < 3; i++) {
        if (i) out.print(',');
        out.print(F("{\"hz\":")); out.print(d.taajuus_hz[i]);
        out.print(F(",\"ok\":")); jsonBool(out, d.taajuusOk[i]);
        out.print('}');
      }
      out.print(']');
    }
    out.print('}');
  }
}

// ===== LÄHETYS (0=Lähetä) =====
void tietokantaLaheta() {
  if (!tietokantaVoiLahettaa()) return;   // ei lähetettävää: nappi ohitetaan

  naytaLahetetaan();
  if (verkkoLahetaTulos()) {
    Serial.println(TXT("Lahetetty", "Sent"));
    tietokantaTyhjenna();                 // ei lähetetä toista kertaa
    naytaLahetetty();
  } else {
    Serial.println(TXT("Lahetys epaonnistui", "Sending failed"));
    naytaLahetysEpaonnistui();            // tulos jää: 0=Yritä uudelleen
  }
}
