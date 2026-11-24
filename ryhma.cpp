#include "ryhma.h"

// ===== TILA =====
// Tyhjä merkkijono = tunnusta ei ole asetettu. Syöttö tulee vaiheessa 3
// (C=Ryhmävalikko).
char ryhmatunnus[RYHMATUNNUS_PITUUS + 1] = "";

bool ryhmaTunnusAsetettu() {
  return ryhmatunnus[0] != '\0';
}

void ryhmaTunnusNaytolle(char* buf, size_t koko) {
  if (ryhmaTunnusAsetettu()) snprintf_P(buf, koko, PSTR("R=%s"), ryhmatunnus);
  else                       snprintf_P(buf, koko, PSTR("R=-------"));
}
