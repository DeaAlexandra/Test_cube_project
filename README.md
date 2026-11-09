# Sääaseman sovelluspiirien testilaite

Arduino Mega 2560 -pohjainen testilaite, joka simuloi sääaseman signaalilaatikkoa.
Laite syöttää testattavalle sovelluspiirille tunnetun jännitteen tai pulssisignaalin,
mittaa piirin lähdön ja ilmoittaa, toimiiko piiri vaatimusten mukaisesti.
Laite on tarkoitettu ensimmäisen vuoden opiskelijoiden rakentamien piirien
tarkastamiseen. Projekti on jatkoa tiimin aiemmalle harjoitustyölle. Nykyisen version
jatkokehityksestä vastaa Alexandra Saarelainen tiimin luvalla.

## Ominaisuudet

- **Analoginen testaus:** lämpötila, valoisuus ja tuulen suunta, viisi valittavaa
  testijännitettä anturia kohden
- **Digitaalinen testaus:** tuulen nopeus, sademäärä sekä sisä- ja ulkoilman kosteus
- **Selkeä tulos:** OK / VIRHE kahdella LCD-näytöllä sekä vihreä tai punainen merkkivalo
- **Vikatunnistus:** oikosulku, avoin piiri ja puuttuva signaali tunnistetaan, ja
  mittaus keskeytetään ilman, että laite jumittuu
- **Kielivalinta:** suomi / englanti
- **Tulossa:** verkkoyhteys (Ethernet + MQTT), mittaustulosten tallennus tietokantaan
  ryhmätunnuksella sekä vikaloki EEPROM-muistiin

## Laitteisto

| Osa | Tehtävä |
|---|---|
| Arduino Mega 2560 | Ohjain |
| BOB (4-kanavainen DAC) | Tuottaa analogiset testijännitteet, ohjataan sarjaportilla (Serial2) |
| Multiplekseri | Valitsee digitaalisen testisignaalin johtimen |
| 2 x LCD 20x4, I2C (osoitteet 0x23 ja 0x27) | Yläruutu: valikot ja tila. Alaruutu: mittaustulokset |
| 4x4-näppäimistö | Käyttäjän syötteet |
| RGB-merkkivalo | Testin tulos (vihreä / punainen) |

## Kytkennät

| Pinni | Portti / bitti | Suunta | Käyttö |
|---|---|---|---|
| 2 | PE4 / INT4 | Tulo | Testattavan piirin digitaalinen lähtö (ulkoinen keskeytys) |
| A0 | PF0 / ADC0 | Tulo | Digitaalisen lähdön jännite |
| A1 | PF1 / ADC1 | Tulo | Analogisen piirin lähtöjännite |
| 3 | PE5 | Lähtö | RGB-merkkivalo, punainen |
| 5 | PE3 | Lähtö | RGB-merkkivalo, vihreä |
| 6 | PH3 | Lähtö | RGB-merkkivalo, sininen |
| 32 | PC5 | Lähtö | Testipulssi: tuulen nopeus |
| 33 | PC4 | Lähtö | Testipulssi: sademäärä |
| 38 | PD7 | Lähtö | Testipulssi: sisäilman kosteus |
| 39 | PG2 | Lähtö | Testipulssi: ulkoilman kosteus |
| 40 | PG1 | Lähtö | Multiplekseri EN (aktiivinen LOW) |
| 41 | PG0 | Lähtö | Multiplekseri S0 |
| 42 | PL7 | Lähtö | Multiplekseri S1 |
| 43 | PL6 | Lähtö | Multiplekseri S2 |
| 16 / 17 | PH1 / PH0 (USART2) | Lähtö / tulo | BOB-DAC (Serial2) |
| 20 / 21 | PD1 / PD0 (TWI) | I2C | LCD-näytöt (SDA / SCL) |
| 22-25 | PA0-PA3 | Tulo | Näppäimistön sarakkeet |
| 26-28, 31 | PA4-PA6, PC6 | Lähtö | Näppäimistön rivit |

## Käyttö

| Näppäin | Toiminto |
|---|---|
| **A** | Analoginen testaus: valitse anturi (1-3) ja testijännite (1-5) |
| **D** | Digitaalinen testaus: valitse kanava (1 = tuuli, 2 = sade, 3 = sisäkosteus, 4 = ulkokosteus) |
| **B** | Vaihda kieli (suomi / englanti) |
| **\*** | Tuloksen jälkeen: uusi mittaus |
| **#** | Paluu päävalikkoon tai käynnissä olevan mittauksen keskeytys |

Digitaalinen mittaus alkaa heti kanavan valinnasta, ja tulos näytetään aina neljän
sekunnin kuluttua. Esimerkki alanäytöstä:

```
TULOS                    TULOS
Lev: 3.3ms      8/10     6.9:OK 7.4:OK 7.9:OK
U=4.91V          OK!     U=4.91V          OK!
*=uusi  #=valikko        *=uusi  #=valikko
```

## Testiparametrit

### Analoginen

| Anturi | Piirin tehtävä | Testijännitteet (mV) | Toleranssi |
|---|---|---|---|
| Lämpötila | Uout = 3 x Uin | 500, 800, 1200, 1500, 1650 | ±100 mV |
| Valoisuus | Uout = 2 x Uin | 620, 900, 1200, 1800, 2250 | ±100 mV |
| Tuulen suunta | Uout = Uin x 1,2 V | 1700, 2200, 2700, 3200, 3900 | ±100 mV |

### Digitaalinen

| Kanava | Testisignaali | Hyväksytään |
|---|---|---|
| Tuulen nopeus | 40 ms pulssi 200 ms välein, 10 kpl | Lähtöpulssin leveys 2-6 ms, vähintään 80 % pulsseista |
| Sademäärä | 150 ms pulssi 300 ms välein, 10 kpl | Lähtöpulssin leveys 3-50 ms, vähintään 80 % pulsseista |
| Kosteus (sisä / ulko) | 50 % neliöaalto 6,9 / 7,4 / 7,9 kHz, kukin 1 s | Lähdön taajuus ±1 %, vähintään 95 % pulsseista |

Kaikilla digitaalisilla kanavilla jokaisen lähtöpulssin tason pitää olla
HIGH ≥ 4,5 V ja LOW ≤ 0,5 V.

## Tekninen toteutus

Ohjelma ohjaa mikro-ohjaimen sisäisiä lohkoja suoraan rekistereillä
Arduino-kirjaston funktioiden sijaan:

| Lohko | Rekisterit | Käyttö |
|---|---|---|
| GPIO | DDRx, PORTx, PINx | Merkkivalo, testipulssit, multiplekseri |
| Ulkoinen keskeytys | EICRB, EIMSK, EIFR | Lähtöpulssin reunojen tunnistus |
| Timer1 (CTC-tila) | TCCR1A/B, TCNT1, OCR1A, TIMSK1, TIFR1 | Testisignaalin tuottaminen |
| AD-muunnin | ADMUX, ADCSRA, ADCSRB, ADC, DIDR0 | Jännitteiden mittaus |

Keskeytyspalvelut:

- `ISR(INT4_vect)`: mittaa lähtöpulssin leveyden ja arvioi pulssin
- `ISR(TIMER1_COMPA_vect)`: tuottaa testisignaalin ja laskee lähetetyt pulssit
- `ISR(ADC_vect)`: kerää lähtösignaalin HIGH- ja LOW-tasot

Lisäksi käytetään USART-lohkoa (BOB-DAC) ja TWI-lohkoa (LCD-näytöt) kirjastojen kautta.

## Kääntäminen

1. Asenna [Arduino IDE](https://www.arduino.cc/en/software).
2. Asenna kirjastot (Library Manager):
   - **Keypad** (Mark Stanley, Alexander Brevig)
   - **LiquidCrystal I2C** (Frank de Brabander)
3. Valitse kortiksi **Arduino Mega or Mega 2560**.
4. Avaa `Mega_4_11_2026_version_2_0.ino` ja lataa ohjelma laitteelle.

Sketsikansion nimen on oltava sama kuin `.ino`-tiedoston nimi.

## Tiedostorakenne

| Tiedosto | Sisältö |
|---|---|
| `Mega_4_11_2026_version_2_0.ino` | `setup()` ja pääsilmukka |
| `adc.cpp / .h` | AD-muuntimen ohjaus ja ADC-keskeytys |
| `analog.cpp / .h` | Analoginen testaus ja BOB-DAC-komennot |
| `digital.cpp / .h` | Digitaalinen testaus: Timer1-generaattori, INT4-mittaus, tilakone |
| `display.cpp / .h` | LCD-näyttöjen näkymät |
| `errormode.cpp / .h` | Vikojen tunnistus ja vikailmoitus |
| `led.cpp / .h` | RGB-merkkivalo |
| `ui.cpp / .h` | Näppäimistö, valikkologiikka ja multiplekseri |
| `lang.cpp / .h` | Kielivalinta |

## Tila ja jatkokehitys

- [x] Porttien, keskeytysten, ajastimen ja AD-muuntimen ohjaus rekistereillä
- [x] Digitaalinen mittaus kiinteällä mittausajalla ja tilastollisella hyväksynnällä
- [x] Vikatila, joka ei pysäytä laitetta
- [ ] Testaus laitteella
- [ ] Vikaloki EEPROM-muistiin
- [ ] Ethernet-yhteys ja MQTT-lähetys
- [ ] Palvelin ja tietokanta mittaustuloksille
- [ ] Ryhmätunnuksen syöttö (C-näppäin)
- [ ] MBED-alustan vaihto Arduino Uno R4:ään BOB-DAC:n ohjaimena

## Tekijät

- Alexandra Saarelainen, pääasiassa koodi ja kokonaisuuden hallinta, nykyinen jatkokehitys
- Kaapo Kyttälä, pääasiassa kotelon mallinnus ja koodaus, ensimmäisen version kehitys
- Mikko Metso, pääasiassa kytkentäsuunnittelu ja dokumentointi, ensimmäisen version kehitys 

## Lisenssi

MIT, ks. [LICENSE](LICENSE).
