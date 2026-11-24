#include "asetukset.h"
#include "display.h"
#include "lang.h"
#include "verkko.h"

// ===== TILA =====
static AsetusTila asetus_tila = ASET_POIS;

void asetuksetAvaa() {
  asetus_tila = ASET_VALIKKO;
  naytaAsetukset();
  Serial.println(TXT("ASETUKSET: 1=tiedot 2=kieli 3=verkko #=valikko",
                     "SETTINGS:  1=info   2=lang  3=network #=menu"));
}

bool asetuksetAktiivinen() {
  return asetus_tila != ASET_POIS;
}

void asetuksetKasittele(char nappi) {
  // '#' palaa päävalikkoon kaikkialta
  if (nappi == '#') {
    asetus_tila = ASET_POIS;
    naytaPaavalikko();
    return;
  }

  // ----- Kielivalinta auki -----
  if (asetus_tila == ASET_KIELI) {
    if (nappi == '1' || nappi == '2') {
      kieli_englanti = (nappi == '2');
      Serial.println(kieli_englanti ? F("Language: English") : F("Kieli: Suomi"));
      asetus_tila = ASET_VALIKKO;
      naytaAsetukset();      // ylänäyttö heti uudella kielellä, alanäyttö tyhjä
    }
    return;                  // muut napit ohitetaan
  }

  // ----- Asetukset-valikko -----
  switch (nappi) {
    case '1':
      naytaLaitteenTiedot();
      break;
    case '2':
      asetus_tila = ASET_KIELI;
      naytaKielivalinta();
      break;
    case '3':
      naytaVerkonTila(verkkoYhdistetty());
      break;
    default:
      break;                 // muut napit ohitetaan
  }
}
