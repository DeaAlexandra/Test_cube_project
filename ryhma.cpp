#include "ryhma.h"
#include "display.h"
#include "lang.h"
#include "tietokanta.h"

// ===== TILA =====
// Tyhjä merkkijono = tunnusta ei ole asetettu.
char ryhmatunnus[RYHMATUNNUS_PITUUS + 1] = "";

static RyhmaTila ryhma_tila = RYHMA_POIS;

// Syötön aikainen puskuri. Valmis tunnus kopioidaan ryhmatunnukseen vasta,
// kun kaikki 7 merkkiä on syötetty, joten keskeytetty syöttö ei muuta
// voimassa olevaa tunnusta.
static char    syotto[RYHMATUNNUS_PITUUS + 1];
static uint8_t syotto_pituus = 0;

bool ryhmaTunnusAsetettu() {
  return ryhmatunnus[0] != '\0';
}

void ryhmaTunnusNaytolle(char* buf, size_t koko) {
  if (ryhmaTunnusAsetettu()) snprintf_P(buf, koko, PSTR("R=%s"), ryhmatunnus);
  else                       snprintf_P(buf, koko, PSTR("R=-------"));
}

// ===== VALIKKO =====
void ryhmaAvaa() {
  ryhma_tila = RYHMA_VALIKKO;
  naytaRyhmavalikko();
  Serial.println(TXT("RYHMA: 1=syota tunnus 2=poista tunnus #=valikko",
                     "GROUP: 1=enter ID     2=delete ID     #=menu"));
}

bool ryhmaAktiivinen() {
  return ryhma_tila != RYHMA_POIS;
}

// Kelpaako nappi syötön seuraavaksi merkiksi? Paikat 0-1 ja 3-6 ovat
// numeroita, paikka 2 on luokan kirjain. Näppäimistön A-D-napit antavat
// kirjaimen suoraan.
static bool merkkiKelpaa(char nappi, uint8_t paikka) {
  if (paikka == 2) return nappi >= 'A' && nappi <= 'D';
  return nappi >= '0' && nappi <= '9';
}

static void kasitteleSyotto(char nappi) {
  if (nappi == '*') {                        // poista viimeisin merkki
    if (syotto_pituus > 0) syotto_pituus--;
    naytaTunnusSyotto(syotto, syotto_pituus);
    return;
  }

  if (!merkkiKelpaa(nappi, syotto_pituus)) return;   // väärä merkki ohitetaan

  syotto[syotto_pituus++] = nappi;

  if (syotto_pituus < RYHMATUNNUS_PITUUS) {
    naytaTunnusSyotto(syotto, syotto_pituus);
    return;
  }

  // Kaikki 7 merkkiä: tallennetaan
  syotto[RYHMATUNNUS_PITUUS] = '\0';
  memcpy(ryhmatunnus, syotto, sizeof(ryhmatunnus));
  tietokantaTyhjenna();       // uusi ryhmä: edellisen ryhmän tulokset pois
  Serial.print(TXT("Ryhmatunnus tallennettu: ", "Group ID saved: "));
  Serial.println(ryhmatunnus);

  ryhma_tila = RYHMA_VALIKKO;
  naytaRyhmavalikko();        // ylänäytön R= päivittyy
  naytaTunnusTallennettu();
}

void ryhmaKasittele(char nappi) {
  switch (ryhma_tila) {
    case RYHMA_SYOTTO:
      if (nappi == '#') {                    // keskeytys: vanha tunnus säilyy
        ryhma_tila = RYHMA_POIS;
        naytaPaavalikko();
        return;
      }
      kasitteleSyotto(nappi);
      return;

    case RYHMA_POISTO:
      if (nappi == '*') {                    // Kyllä
        // Myös lähettämättömät tulokset poistetaan, jotta ne eivät jää
        // edellisen ryhmän tunnukselle.
        ryhmatunnus[0] = '\0';
        tietokantaTyhjenna();
        Serial.println(TXT("Ryhmatunnus poistettu", "Group ID deleted"));
        ryhma_tila = RYHMA_VALIKKO;
        naytaRyhmavalikko();                 // R=------- ylänäytölle
        naytaTunnusPoistettu();
      } else if (nappi == '#') {             // Ei: takaisin Ryhmävalikkoon
        ryhma_tila = RYHMA_VALIKKO;
        lcd2.clear();
      }
      return;

    case RYHMA_VALIKKO:
      if (nappi == '#') {
        ryhma_tila = RYHMA_POIS;
        naytaPaavalikko();
      } else if (nappi == '1') {
        syotto_pituus = 0;
        ryhma_tila = RYHMA_SYOTTO;
        naytaTunnusSyotto(syotto, syotto_pituus);
      } else if (nappi == '2') {
        if (ryhmaTunnusAsetettu()) {
          ryhma_tila = RYHMA_POISTO;
          naytaTunnusPoistoVahvistus();
        } else {
          naytaEiTunnusta();
        }
      }
      return;

    case RYHMA_POIS:
      return;
  }
}
