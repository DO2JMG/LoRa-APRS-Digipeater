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

<u>LoRa Module</u><br>
<code>#define LoRa_SCK 5</code><br>
<code>#define LoRa_MISO 19</code><br>
<code>#define LoRa_MOSI 27</code><br>
<code>#define LoRa_SS 18</code><br>
<code>#define LoRa_RST 14</code><br>
<code>#define LoRa_DIO0 26</code><br>

