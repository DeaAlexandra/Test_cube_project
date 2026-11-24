#include "verkko.h"
#include "tietokanta.h"

bool verkkoYhdistetty() {
  return false;   // Ethernet puuttuu vielä
}

// TYNKÄ: tulostaa viestin sarjamonitoriin samassa muodossa, jossa se
// myöhemmin lähetetään MQTT:llä. Oikeassa versiossa tähän tulee
// esimerkiksi mqtt.beginPublish(...), tietokantaKirjoitaJson(mqtt) ja
// mqtt.endPublish().
bool verkkoLahetaTulos() {
  Serial.print(F("[MQTT] "));
  tietokantaKirjoitaJson(Serial);
  Serial.println();
  return VERKKO_SIMULOI_ONNISTUMINEN;
}
