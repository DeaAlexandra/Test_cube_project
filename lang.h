#pragma once
#include <Arduino.h>

extern bool kieli_englanti;

// Palauttaa fi tai en sen mukaan, kumpi kieli on valittuna
inline const char* txt(const char* fi, const char* en) {
  return kieli_englanti ? en : fi;
}
