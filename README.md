# LoRa APRS Digipeater

<b>Supported boards</b>
- Heltec WiFi LoRa 32 V1 (433MHz SX1278)
- Heltec WiFi LoRa 32 V2 (433MHz SX1278)
- TTGO T-Beam V0.7 (433MHz SX1278)
- TTGO T-Beam V1 (433MHz SX1278)
- TTGO Lora32 v2.1
- TTGO Lora32 v2

<b>Path that will digipeated</b><br>
- WIDE1-1 or more
- DST Call with -1

<b>Path that will not digipeated</b><br>
- Own Call
- NOGATE
- RFONLY
- DST-Path an WIDE in one Path
- 
<b>Require LoRa module pinout definitions and Display pinout in</b> <code>config.h</code>:

<u>LoRa Module</u><br>
<br>
<code>#define LoRa_SCK 5</code><br>
<code>#define LoRa_MISO 19</code><br>
<code>#define LoRa_MOSI 27</code><br>
<code>#define LoRa_SS 18</code><br>
<code>#define LoRa_RST 14</code><br>
<code>#define LoRa_DIO0 26</code><br>
<br>
<u>Display</u><br>
<br>
<code>#define LoRa_Oled_SDA 21</code><br>
<code>#define LoRa_Oled_SCL 22</code><br>
<code>#define LoRa_Oled_RST 16</code><br>
<br>
