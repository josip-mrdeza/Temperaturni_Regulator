# Temperaturni Regulator - STM32 Firmware

Sustav za nadzor i regulaciju temperature i vlažnosti zraka baziran na mikrokontroleru STM32F411CEU6 (Black Pill). Uređaj periodički očitava digitalni senzor DHT22, prikazuje podatke i izbornike na lokalnom I2C OLED zaslonu, upravlja mikrosrvom za zaklopku zraka, signalizira odstupanja putem LED dioda te omogućuje dvosmjernu komunikaciju s računalom preko USB CDC protokola (Virtual COM Port).

Projekt je izrađen u sklopu kolegija Ugradbeni računalni sustavi (URS) na Tehničkom veleučilištu u Zagrebu (TVZ) - Prijediplomski strucni studij Mehatronike.

Autori: Josip Mrdeža i Noa Ivićek

---

## Desktop aplikacija (Qt GUI)

Za praćenje telemetrije i konfiguraciju s računala razvijena je prateća aplikacija u Qt 6 (C++).

Izvorni kod, upute za desktop aplikaciju te compiled binary nalaze se u repozitoriju:  
https://github.com/josip-mrdeza/Temperaturni_Regulator_Qt

### Protokol komunikacije s aplikacijom
Komunikacija s računalom odvija se preko USB kabela (USB CDC Virtual COM Port) bez potrebe za eksternim USB-UART pretvaračima:
- STM32 periodički šalje trenutne podatke u formatu:
  - `T:<temperatura>\n` (npr. `T:23.50\n`)
  - `H:<vlaznost>\n` (npr. `H:48.20\n`)
- Desktop aplikacija šalje naredbe mikrokontroleru:
  - `T:<vrijednost>` - postavljanje ciljane temperature i prebacivanje na temperaturni meni
  - `H:<vrijednost>` - postavljanje ciljane vlažnosti i prebacivanje na meni vlažnosti
  - `MenuTemp` - otvaranje izbornika temperature na OLED zaslonu
  - `MenuHum` - otvaranje izbornika vlažnosti na OLED zaslonu
  - `MenuPower` - otvaranje prikaza napona, struje, snage i interne temperature čipa
  - `MenuUsb` - prikaz zadnjih primljenih sirovih bajtova s USB-a (debug pregled)

Prijem podataka na mikrokontroleru izveden je asinkrono u `CDC_Receive_FS()` rutini pomoću međuspremnika (buffer) i flag-a (`usb_rx_buff_flag`), čime se izbjegava blokiranje glavne petlje i prekidnih rutina.

---

## Hardver i periferija

- **Mikrokontroler:** STM32F411CEU6 (ARM Cortex-M4 @ 96 MHz, 512 KB Flash, 128 KB SRAM)
- **Senzor temperature i vlage:** DHT22 (AM2302)
- **Zaslon:** 0.96" I2C OLED zaslon (SSD1306 / SSD1315 kontroler, rezolucija 128x64)
- **Aktuator:** Mikroservo motor MG90S s metalnim zupčanicima (radni kut 0°–180°)
- **Indikacija:** 3x LED diode (crvena, žuta, zelena)
- **Tipke:** 2x push-button tipkala za lokalnu navigaciju
- **Mjerenje napajanja:** Shunt otpornik i analogni ulazi na STM32 ADC1

---

## Raspored pinova (Pinout)

| Oznaka pina | Funkcija / Spojeni hardver | Opis |
| :--- | :--- | :--- |
| PA0 | ADC1_IN0 | Mjerenje sistemskog napona preko razdjelnika |
| PA1 | ADC1_IN1 | Mjerenje struje preko shunt otpornika |
| PA2 | Btn_Back | Tipka za povratak / navigaciju |
| PA3 | Btn_FW | Tipka za potvrdu / kretanje naprijed |
| PA5 | GPIO Output | Opći statusni izlaz |
| PA11 | USB_OTG_FS_DM | USB Data- linija (Type-C konektor na pločici) |
| PA12 | USB_OTG_FS_DP | USB Data+ linija (Type-C konektor na pločici) |
| PA15 | TIM2_CH1 | PWM izlaz za MG90S servo (50 Hz) |
| PB0 | DHT22 DATA | Dvosmjerna podatkovna linija senzora (open-drain s pull-upom) |
| PB4 | LED_RED | Crvena LED - grijanje (trenutna temperatura < ciljana) |
| PB5 | LED_YELLOW | Žuta LED - unutar tolerancije (|ciljana - trenutna| < 1 °C) |
| PB6 | LED_GREEN | Zelena LED - hlađenje (trenutna temperatura > ciljana) |
| PB10 | I2C2_SCL | I2C takt za OLED zaslon |
| PB11 | I2C2_SDA | I2C podaci za OLED zaslon |

---

## Funkcionalnosti firmvera

### 1. Čitanje senzora DHT22
Komunikacija sa senzorom odvija se preko jedne linije pomoću prilagođenog vremenskog protokola:
1. Mikrokontroler šalje start signal povlačenjem linije u logičku nulu na 1.2 ms, nakon čega oslobađa liniju.
2. Senzor šalje odziv i 40 bitova podataka (16 bita vlaga, 16 bita temperatura, 8 bita paritet).
3. Mjerenje trajanja impulsa radi se hardverskim brojačem tajmera TIM3 bez blokirajućih HAL delay funkcija.
4. Nakon uspješne provjere kontrolnog zbroja podaci se spremaju u strukturu `DHT22_t` i šalju na prikaz i USB.

### 2. Regulacija i indikatori
- **Servomotor MG90S:** Tajmer TIM2 konfiguriran je za generiranje PWM signala frekvencije 50 Hz (perioda 20 ms). Širina impulsa od ~0.5 ms do ~2.33 ms mapira se u raspon kuta zakreta od 0° do 180° za pozicioniranje ventilacijske klapne.
- **LED diode:**
  - Crvena (PB4): uključena kada je izmjerena temperatura niža od ciljane.
  - Žuta (PB5): uključena kada je razlika između ciljane i izmjerene temperature manja od 1 °C.
  - Zelena (PB6): uključena kada je izmjerena temperatura viša od ciljane.

### 3. OLED zaslon i izbornici
Zaslon se osvježava preko I2C2 sabirnice pri 100 kHz. Podržani su sljedeći prikazi:
- `STATE_TEMP_ADJUST`: trenutna temperatura, zadana temperatura i razlika (delta).
- `STATE_HUM_ADJUST`: trenutna vlažnost, zadana vlažnost i razlika.
- `STATE_POWER`: ulazni napon (V), struja sustava (A), izračunata snaga (W) i temperatura STM32 čipa očitanje s internog senzora na ADC1.
- `STATE_USB_RX`: debug ispis zadnjeg tekstualnog paketa primljenog s računala.
- `STATE_INIT`: provjera komponenti pri pokretanju (DHT22, tajmeri, ADC, USB i servo test).

### 4. Mjerenje analognih veličina (ADC1)
ADC1 je postavljen na 12-bitnu rezoluciju s diskontinuiranom konverzijom triju kanala:
- Kanal 0: sistemski napon
- Kanal 1: pad napona na mjernom otporniku za izračun struje
- Interni temperaturni senzor: izračun temperature silicija prema STM32 formuli: `((Vsense - 760 mV) / 2.5 mV/°C) + 25 °C`

---

## Struktura direktorija

```plaintext
Temperaturni_Regulator/
├── Core/
│   ├── Inc/
│   │   ├── LCD/ssd1315.h          # Definicije i funkcije za OLED zaslon i menije
│   │   ├── Sensors/dht22.h        # Deklaracija strukture i funkcija za DHT22
│   │   ├── adc.h, gpio.h, i2c.h   # Konfiguracijska zaglavlja periferije
│   │   ├── tim.h, main.h          # Tajmeri i definicije pinova
│   └── Src/
│       ├── LCD/ssd1315.c          # Iscrtavanje grafičkih elemenata i teksta
│       ├── Sensors/dht22.c        # Timing i očitavanje DHT22 senzora
│       ├── adc.c, gpio.c, i2c.c   # Inicijalizacija i rad s periferijom
│       ├── tim.c                  # TIM2 (PWM servo) i TIM3 konfiguracija
│       └── main.c                 # Glavna radna petlja i logika regulacije
├── USB_DEVICE/                    # USB stack za Virtual COM Port (CDC klasa)
├── Drivers/                       # STM32F4xx HAL i CMSIS biblioteke
├── Temperaturni_Regulator.ioc     # STM32CubeMX projektna konfiguracija
└── README.md                      # Dokumentacija projekta
```

---

## Rad s projektom

1. Otvoriti projekt u **STM32CubeIDE** (verzija 1.14 ili novija).
2. Spojiti pločicu preko ST-Link programatora na SWD pinove (SWDIO, SWCLK, GND, 3V3) ili putem USB DFU načina rada.
3. Pokrenuti prevođenje projekta (`Project -> Build Project` ili `Ctrl + B`).
4. Učitati binarnu datoteku na mikrokontroler (`Run -> Run` ili `F11` za debug).
5. Nakon pokretanja uređaj izvodi self-test servomotora i senzora te prelazi u radni mod.
6. Za kontrolu s računala spojiti USB kabel na Type-C konektor Black Pill pločice i pokrenuti aplikaciju `Temperaturni_Regulator_Qt`.
