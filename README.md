# LoRa APRS Digipeater

<b>Supported boards</b>
- Heltec WiFi LoRa 32 V1 (433MHz SX1278)
- Heltec WiFi LoRa 32 V2 (433MHz SX1278)
- TTGO T-Beam V0.7 (433MHz SX1278)
- TTGO T-Beam V1 (433MHz SX1278)
- TTGO Lora32 v2.1
- TTGO Lora32 v2

Only WIDE1-1 or Destinationcall with -1 will digipeated.

<b>Require LoRa module pinout definitions and Display pinout in</b> <code>config.h</code>:

<u>Lora</u>
<code>
#define LoRa_SCK 5\n
#define LoRa_MISO 19<br>
#define LoRa_MOSI 27<br>
#define LoRa_SS 18<br>
#define LoRa_RST 14<br>
#define LoRa_DIO0 26<br>
</code>

<u>Display</u>
<code>
#define LoRa_Oled_SDA 21<br>
#define LoRa_Oled_SCL 22<br>
#define LoRa_Oled_RST 16<br>  
</code>
