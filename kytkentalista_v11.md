# Kytkentälista – arduino koko piirikaavio v11

Lähde: `arduino koko piirikaavio v11.kicad_sch`. Netlista on viety kicad-cli:llä (KiCad 10.0) 13.11.2026.

**Näin listaa luetaan**

- "J7 nn" kaaviossa = Arduino Megan digitaalipinni nn.
- Kaaviossa Arduinon pää ja komponentin pää on yhdistetty **nimilapuilla**. Useimmissa kohdissa lapuilla on eri nimi eri päissä (esim. "M_plekseri E" ja "J7 39"). KiCadin mielestä ne eivät siksi ole sama johto. Alla olevat parit on koottu lappujen merkityksen perusteella.
- Sarake **Ohjelma** kertoo, mitä toimiva ohjelma (`Mega_4_11_2026_version_2_0`) odottaa. ⚠️ = kaavio ja ohjelma eroavat, tai kaavio on ristiriidassa itsensä kanssa.
- MCP6022 (8-jalkainen): 1 = lähtö, 2 = − tulo, 3 = + tulo, 4 = V− (GND), 8 = V+ (+5 V).

---

## 1. Arduino-pinni → mihin (kaavion mukaan)

### Analogiapinnit (liitin J3)

| Arduino | J3 jalka | Lappu Arduinon päässä | Komponentin pää | Ohjelma |
|---|---|---|---|---|
| A0 | 1 | D_Pulssi | U17 jalka 1 (piiri 7, "arduino A0" / D_jännite) | A0 = D_jännite ⚠️ |
| A1 | 2 | A_mittaus jännite piiri 6 | U16 jalka 1 (piiri 6, "Arduino A1") | A1 ✓ |
| A4 | 5 | D_jännite | – (vastinetta ei ole) | ei käytössä ⚠️ |

### PWM-pinnit (liittimet J2, J4)

| Arduino | Liitin.jalka | Lappu | Komponentin pää | Ohjelma |
|---|---|---|---|---|
| 3 | J4.5 | LED R | RGB-ledi D5 jalka 2 (R) | 3 ✓ |
| 5 | J4.3 | LED G | RGB-ledi D5 jalka 3 (G) | 5 ✓ |
| 6 | J4.2 | LED B | RGB-ledi D5 jalka 1 (B) | 6 ✓ |
| 11 | J2.7 | E | – | ei käytössä |
| 12 | J2.6 | RS | – | ei käytössä |
| 2 | J4.6 | – (ei kytketty) | – | **D_pulssi (INT4)** ⚠️ |
| GND | J2.4 | GND | GND | |

### Tiedonsiirto (liitin J6)

| Arduino | J6 jalka | Lappu | Komponentin pää | Ohjelma |
|---|---|---|---|---|
| 16 TX2 | 3 | BOB pohjalevy TX | BOB-pohjalevy J15 TX | Serial2 ✓ (tarkista TX→RX) |
| 17 RX2 | 4 | BOB pohjalevy RX | BOB-pohjalevy J15 RX | Serial2 ✓ |
| 20 SDA | 7 (ja J2.2) | SDA Jakaja1 (piiri 9) | Jakaja U3 jalka 1 (SDA sisään) | ✓ |
| 21 SCL | 8 (ja J2.1) | SCL Jakaja2 (piiri 9) | Jakaja U3 jalka 2 (SCL sisään) | ✓ |

### Digitaalipinnit (liitin J7)

| Arduino | J7 jalka | Lappu Arduinon päässä | Komponentin pää (lappu) | Ohjelma |
|---|---|---|---|---|
| 22 | 33 | R1 | Näppäimistö U2 jalka 1 ("J7 22") | 22 ✓ |
| 23 | 34 | R2 | Näppäimistö U2 jalka 2 ("J7 23") | 23 ✓ |
| 24 | 31 | R3 | Näppäimistö U2 jalka 3 ("J7 24") | 24 ✓ |
| 25 | 32 | R4 | Näppäimistö U2 jalka 4 ("J7 25") | 25 ✓ |
| 26 | 29 | C1 | Näppäimistö U2 jalka 5 ("J7 26") | 26 ✓ |
| 27 | 30 | C2 | Näppäimistö U2 jalka 6 ("J7 27") | 27 ✓ |
| 28 | 27 | C3 | Näppäimistö U2 jalka 7 ("J7 28") | 28 ✓ |
| 29 | 28 | C4 | Näppäimistö U2 jalka 8 ("J7 29") | **31** ⚠️ |
| 32 | 23 | – | – | tuulen testipulssi ⚠️ |
| 33 | 24 | – | – | sateen testipulssi ⚠️ |
| 35 | 22 | – | U17 jalka 1 ("J7 35", D_pulssi) | ei käytössä ⚠️ |
| 36 | 19 | Vastus R5 | R14 15k yläpää ("J7 36") | – ⚠️ ks. kohta 3 |
| 37 | 20 | Vastus R3 | R17 18k yläpää ("J7 37") | – ⚠️ |
| 38 | 17 | Vastus R1 | R18 10k yläpää ("J7 38") | sisäkosteus |
| 39 | 18 | Vastus R7 | R20 3,9k yläpää **ja** 74HC4051 jalka 6 (E) ("J7 39") | ulkokosteus ⚠️ |
| 40 | 15 | M_plekseri E | 74HC4051 jalka 11 (S0) ("J7 40") | E ⚠️ |
| 41 | 16 | M_plekseri S0 | 74HC4051 jalka 10 (S1) ("J7 41") | S0 ⚠️ |
| 42 | 13 | M_plekseri S1 | 74HC4051 jalka 9 (S2) ("J7 42") | S1 ⚠️ |
| 43 | 14 | M_plekseri S2 | – | S2 ⚠️ |
| 44 | 11 | MCP6022 3 | – (vastinetta ei ole) | ei käytössä |
| 5V | 35, 36 | +5V | taustavalon R1–R4, näytöt U14/U15 jalka 2 | |
| GND | 1, 2 | GND | yhteinen GND | |

### Virta (liitin J1)

| Arduino | J1 jalka | Lappu | Mihin |
|---|---|---|---|
| 5V | 5 | +5V BOB pohjalevy 5V0 | BOB-pohjalevy J13 5V0 |
| 3V3 | 4 | +3V3 | – |
| GND | 6, 7 | GND BOB pohjalevy GND | BOB-pohjalevy J13 GND ⚠️ (eri nimi kuin GND, ei yhdisty kaaviossa yhteiseen maahan) |

---

## 2. Signaalitiet banaaniliittimiltä

### Piiri 7 – Digitaalinen mittaus

| Mistä | Mihin |
|---|---|
| Banaani **Digital input** | U17 jalka 3 (+) |
| U17 jalka 1 | U17 jalka 2 |
| U17 jalka 1 | Arduino A0 ja "J7 35" (ohjelma: A0 ja **pinni 2**) |

### Piiri 6 – Analoginen mittaus

| Mistä | Mihin |
|---|---|
| Banaani **Analoginen mittauspiste** | U16 jalka 3 (+) |
| U16 jalka 1 | U16 jalka 2 ja Arduino A1 |

### Piiri 4 ja piiri 3 – BOB:n analogialähdöt

| Mistä | Mihin | Ohjelma |
|---|---|---|
| BOB (MAX5715BOB) H1 jalka 2 **OUTA** | U5 jalka 3 (+) | OUTA = testijännite Vin ⚠️ |
| U5 jalka 1 | U5 jalka 2 ja banaani **Analoginen REF jännite** | REF pitäisi tulla OUTB:stä ⚠️ |
| BOB H1 jalka 3 **OUTB** | U4 jalka 3 (+) | OUTB = 1,2 V ⚠️ |
| U4 jalka 1 | U4 jalka 2 ja banaani **Analoginen syöttö** | syöttö pitäisi tulla OUTA:sta ⚠️ |
| BOB H1 jalka 4 GND | GND | |
| BOB H1 jalka 1 REF | ei kytketty (kaavion huomautus) | |
| U4 jalka 8, U5 jalka 8 | MB102 5V | |
| U4 jalka 4, U5 jalka 4 | **ei kytketty** ⚠️ | pitää olla GND |

### Piiri 5 – Digitaalinen testisignaali

| Mistä | Mihin |
|---|---|
| "J7 36" | R14 15k → keskipiste → R15 10k → GND |
| R14/R15-keskipiste | U10 jalka 3 (+) |
| U10 jalka 1 | U10 jalka 2 ja 74HC4051 jalka 1 (A4) |
| "J7 37" | R17 18k → keskipiste → R16 10k → GND |
| R17/R16-keskipiste | U11 jalka 3 (+) |
| U11 jalka 1 | U11 jalka 2 ja 74HC4051 jalka 14 (A1) |
| "J7 38" | R18 10k → keskipiste → R19 4,7k → GND |
| R18/R19-keskipiste | U12 jalka 3 (+) |
| U12 jalka 1 | U12 jalka 2 ja 74HC4051 jalka 15 (A2) |
| "J7 39" | R20 3,9k → keskipiste → R21 1k → GND |
| R20/R21-keskipiste | U13 jalka 3 (+) |
| U13 jalka 1 | U13 jalka 2 ja 74HC4051 jalka 12 (A3) |
| 74HC4051 jalka 3 (A, yhteinen) | Banaani **Digital Output (D_OUT)** |
| 74HC4051 jalka 16 (VCC) | MB102 5V |
| 74HC4051 jalka 8 (GND) | GND |
| 74HC4051 jalka 7 (VEE) | **ei kytketty** ⚠️ pitää olla GND |
| 74HC4051 jalat 2, 4, 5, 13 (A6, A7, A5, A0) | ei kytketty (käyttämättömät kanavat) |

Ohjelma valitsee kanavat näin: tuuli = kanava 4 (U10), sade = 1 (U11), sisä = 2 (U12), ulko = 3 (U13). Tämä täsmää kaavioon.

---

## 3. Muut piirit

### Piiri 9 – Näytöt

| Mistä | Mihin |
|---|---|
| Jakaja U3 lähtö 1 (SDA) | Ylempi näyttö U14 jalka 3 (SDA) |
| Jakaja U3 lähtö 2 (SDA) | Alempi näyttö U15 jalka 3 (SDA) |
| Jakaja U3 lähtö 3 (SCL) | Ylempi näyttö U14 jalka 4 (SCL) |
| Jakaja U3 lähtö 4 (SCL) | Alempi näyttö U15 jalka 4 (SCL) |
| U14 jalka 1, U15 jalka 1 | GND |
| U14 jalka 2, U15 jalka 2 | +5V |

### Piiri 13 – Taustavalo ja merkkivalo

| Mistä | Mihin |
|---|---|
| +5V | R1, R2, R3, R4 (220 Ω) jalka 1 |
| R1 / R2 / R3 / R4 jalka 2 | LED D3 / D4 / D6 / D7 anodi |
| D3, D4, D6, D7 katodi | GND |
| RGB-ledi D5 jalka 4 (yhteinen katodi) | **ei kytketty** ⚠️ pitää olla GND |

### Piiri 10, 1 ja 2 – Virransyöttö

| Mistä | Mihin |
|---|---|
| Virtalähde U6 jalka 4 (+Vout) | U7 jalka 2 (+ sisään) |
| Virtalähde U6 jalka 3 (−Vout) | U7 jalka 1 (− sisään) |
| U7 +9V | Virtakytkin SW1 jalka 1 (A) |
| SW1 jalka 2 (B) | U8 jalka 2 (+ sisään) |
| U7 − | U8 jalka 1 (− sisään) |
| U8 +9V | MB102 + DC_IN |
| U8 − | MB102 DC_IN − (lappu "DC_IN") |
| MB102 5V | U4 jalka 8, U5 jalka 8 (ja lapulla "MB102 5V": 74HC4051 jalka 16) |
| MB102 Ground | GND |
| SW1 jalka 3 (C) | ei kytketty |

---

## 4. Kaavion virheet ja puutteet, jotka netlista paljasti

1. **U4 ja U5 jalka 4 (V−) ei ole kytketty GND:hen.** Nämä ovat analogisten lähtöjen (syöttö ja REF) puskurit. Jos maa puuttuu myös oikeasta laitteesta, oparin lähtö voi nousta liian korkealle ja ryömiä. Tämä sopii REF-jännitteen oireeseen. **Tarkista ensimmäisenä.**
2. **OUTA ja OUTB ovat ohjelmaan nähden ristissä**: kaaviossa REF tulee OUTA:sta, ohjelma lähettää 1,2 V:n OUTB:hen.
3. **Lappu "J7 39" yhdistää ulkokosteuden testipulssin (R20) ja multiplekserin E-jalan.** Testipulssi kytkisi multiplekserin päälle ja pois. Arduinon puolella E on pinnissä 40.
4. **Multiplekserin ohjaus on yhden pinnin verran siirtynyt** piiri 5:ssä (S0 = "J7 40" jne.). J7-liitinpiirros ja ohjelma: E = 40, S0 = 41, S1 = 42, S2 = 43.
5. **74HC4051 jalka 7 (VEE) ei ole kytketty.** Sen pitää olla GND.
6. **RGB-ledin yhteinen katodi (D5 jalka 4) ei ole kytketty.**
7. **Oparien U10–U13, U16 ja U17 virtajalkoja (8 ja 4) ei ole piirretty ollenkaan.** Kaaviossa on virtaosa (U4C, U5C) vain U4:lle ja U5:lle.
8. **D_pulssi**: kaaviossa Arduinon pinni 35, ohjelmassa pinni 2. Pinni 2 on kaaviossa tyhjä.
9. **Testipulssit**: kaaviossa tuuli ja sade tulevat pinneistä 36 ja 37, ohjelmassa 32 ja 33. J7-liitinpiirroksen nimet (Vastus R1/R3/R5/R7) eivät myöskään täsmää piiri 5:n pinnimerkintöihin 36 ja 38.
10. **Näppäimistön C4**: kaaviossa pinni 29, ohjelmassa 31.
11. **D_jännite / D_Pulssi** ovat J3:ssa A4 ja A0, piiri 7:ssä A0 ja "J7 35".
12. **Arduinon GND (J1 jalat 6 ja 7)** on eri nimellä ("GND BOB pohjalevy GND") kuin yhteinen GND, joten kaaviossa ne eivät yhdisty.
