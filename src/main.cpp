#include "config.h"
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TimeLib.h>
#include <esp_task_wdt.h>

String LoRa_incoming_Data;

Adafruit_SSD1306 display(Lora_Screen_Width, LoRa_Screen_Height, &Wire, LoRa_Oled_RST);

long l_Timer_Bake = 0;
long l_Timer_Bake_Send = ((1000*60)*LoRa_Timer_Bake);

long l_Timer_Display = 0;
long l_Timer_Display_off = ((1000*60)*Lora_Screen_Timeout);

void LoRa_send(String LoRa_str_Data, int LoRa_i_Header);
void LoRa_init_display();
void LoRa_init();
void LoRa_display(String LoRa_str_display, int LoRa_i_X, int LoRa_i_Y);
String LoRaNumberFormat(unsigned int number, unsigned int width);
String LoRaGetTime(time_t t);

static time_t LoRaNextBeacon = 0;
static time_t LoRaLastBeacon = 0;

void setup() {
  esp_task_wdt_init(LoRa_Watchdog_Timeout, true);     //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);                             //add current thread to WDT watch

  LoRa_init_display();
  LoRa_init();

  Serial.begin(LoRa_Serial_Baudrate);

  esp_task_wdt_reset();

  String loRa_str_Bake = (String)LoRa_str_call + ">" + String(LoRa_str_Dest) + ":!" + (String)LoRa_str_Lat + (String)LoRa_str_Overlay + (String)LoRa_str_Lon + (String)LoRa_str_Symbol + (String)LoRa_str_Comment;
  LoRa_send(loRa_str_Bake, 1);
  LoRa_display("Send Bake",0,20);
  delay(3000);
  LoRaLastBeacon = now();
}

void loop() {
  LoRaNextBeacon =   (LoRa_Timer_Bake*60) - (now() -LoRaLastBeacon);

  LoRa_display("Next Bake : "+LoRaGetTime(LoRaNextBeacon),0,20);
 
  if (millis() > l_Timer_Bake_Send + l_Timer_Bake ) {
    l_Timer_Bake = millis();
    
    String loRa_str_Bake = (String)LoRa_str_call + ">" + String(LoRa_str_Dest) + ":!" + (String)LoRa_str_Lat + (String)LoRa_str_Overlay + (String)LoRa_str_Lon + (String)LoRa_str_Symbol+(String)LoRa_str_Comment;
    LoRa_send(loRa_str_Bake, 1);
    l_Timer_Display = millis();
    display.dim(false);
    LoRa_display("Send Bake",0,20);
    delay(3000);
    LoRaLastBeacon = now();
  }

  if (millis() > l_Timer_Display_off + l_Timer_Display ) {
    l_Timer_Display = millis();
    display.dim(true);
  }
  
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    

    
    display.dim(false);
    l_Timer_Display = millis();
    LoRa_display("Received packet",0,20);
    
    String LoRaHeader;
    String sourceCall;
    String destCall;
    String message;
    String digiPath;
    String digis[8];

    while (LoRa.available()) {

      LoRa_incoming_Data = LoRa.readString();

      if (LoRa_incoming_Data.length() < 5)
        goto bad_packet;
      int pos1, pos2;
      LoRaHeader = LoRa_incoming_Data.substring(0, 3);
      pos1 = LoRa_incoming_Data.indexOf('>');
      if (pos1 < 5)
        goto bad_packet;
      sourceCall = LoRa_incoming_Data.substring(3, pos1);
      pos2 = LoRa_incoming_Data.indexOf(':');
      if (pos2 < pos1)
        goto bad_packet;
      destCall = LoRa_incoming_Data.substring(pos1 + 1, pos2);
      message = LoRa_incoming_Data.substring(pos2 + 1);
      digiPath = "";
      pos2 = destCall.indexOf(',');
      if (pos2 > 0) {
        digiPath = destCall.substring(pos2 + 1);
        destCall = destCall.substring(0, pos2);
      }
      if (destCall == "")
        goto bad_packet;


      // In LORA we don't support digipeating more than one time -> If somewhere in the path the digipeated marker '*' appears, don't repeat.
      if (digiPath.indexOf('*') != -1)
        goto already_repeated;
        
      bool via_digi_found;
      via_digi_found = false;
      pos2 = destCall.indexOf('-');
      if (pos2 > -1) {
        // DST Call digipeating?
        // DST-n>WIDEm is mutually exclusive
        if (digiPath.startsWith("WIDE") || digiPath.indexOf(",WIDE") > -1)
          goto bad_packet;
        // v this makes ^ this obsolete. If digiPath contains a real digi call for digipeating (and it has no repeated flad
        //                (but we well not repeat already repeated packets anywa..), it's also mutually exclusive to DST-Digipeating
        if (!(digiPath == "" || digiPath == "NOGATE" || digiPath == "RFONLY"))
          goto bad_packet;
        int ssid = destCall.substring(pos2 + 1).toInt();
        if (ssid < 1 || ssid > 7) {
          // SSID 8-15: unsupported. < 1 and > 15 are invalid
          goto bad_packet;
        }
        // Decrement SSID; if SSID was -1, then remove "-1".
        destCall = destCall.substring(0, pos2);
        if (ssid > 1)
          destCall = destCall + "-" + String(ssid - 1);
        via_digi_found = true;
      }


      int n_digis;
      int call_start;
      int call_end;
      bool at_last_digi;
      bool my_call_was_in_path;
      n_digis = 0;
      call_start = 0;
      call_end = 0;
      at_last_digi = false;
      my_call_was_in_path = false;

      while (true) {
        String digi;
        call_end = digiPath.indexOf(',', call_start);
        if (call_end == -1) {
          digi = digiPath.substring(call_start);
          at_last_digi = true;
        } else {
          if (n_digis == 6)
            goto max_digipeaters_reached;
          digi = digiPath.substring(call_start, call_end);
        }

        if (digi.startsWith("WIDE")) {
          // In LoRa, we repeat only once. Set WIDEn-m to WIDEn (-> cut off '-'), on every word "WIDEn" in the path
          int ssid_pos;
          if ((ssid_pos = digi.indexOf("-")) > -1)
            digi = digi.substring(0, ssid_pos);
          if (!via_digi_found)
            digi = digi + "*";
          via_digi_found = true;       
        } else {
          /* user specified a digipeater, i.e. ...>APRS,DL2BBB" or >APRS,DL2BBB,WIDE2-1, we must not repeat this. Except if our own call is addressed.
           * Thanks to !via_digi_found status we know we are in the position of digiPath before the first WIDE digi.
           */
          if (digi == String(LoRa_str_call)) {
            if (!via_digi_found) {
              digi = digi + "*";
              my_call_was_in_path = true;
            }
            via_digi_found = true;
          } else {
            if (!via_digi_found)
              goto no_via_digi_found;
          }
        }

        if (digi != "") {
          digis[n_digis] = digi;
          n_digis++;
        }

        if (at_last_digi)
          break;
          
        call_start = call_end + 1;
      }

      if (!via_digi_found)
        goto no_via_digi_found;

      // Now, look for the first reference of wide* and insert our callsign just before
      int i;
      int my_digi_pos;
      my_digi_pos = 0;
      if (!my_call_was_in_path) {
        for (i = n_digis-1; i >= 0; i--) {
          if (digis[i].endsWith("*") || i == 0) {
            my_digi_pos = i;
            int j;
            for (j = n_digis+1; j > i; j--)
              digis[j] = digis[j-1];
          }
        }

        // always add our repeater
        digis[my_digi_pos] = String(LoRa_str_call) + "*";
        n_digis++;
      }

      // rebuild digiPath
      digiPath = "";
      for (i = 0; i < n_digis; i++)
          digiPath = digiPath + "," + digis[i];

      
      LoRa_incoming_Data = LoRaHeader + sourceCall + ">" + destCall + digiPath + ":" + message;
      LoRa_send(LoRa_incoming_Data,0);
      LoRa_display(String(sourceCall + " repeated!"), 0, 20);
      delay(2000);

bad_packet:
      // ignore bad packet
no_via_digi_found:
      // no via digi found
max_digipeaters_reached:
      // max. 8 digipeaters are allowed
already_repeated:
      // already_repeated
      ;

    }
  }
  esp_task_wdt_reset();
}

void LoRa_send(String LoRa_str_Data, int LoRa_i_Header) {
  LoRa.setFrequency(LoRa_Frequency_TX);
  LoRa.beginPacket();
  if (LoRa_i_Header > 0) {
    LoRa.write('<');
    LoRa.write(0xFF);
    LoRa.write(0x01);
  }
  LoRa.write((const uint8_t *)LoRa_str_Data.c_str(), LoRa_str_Data.length());
  LoRa.endPacket();
  LoRa.setFrequency(LoRa_Frequency_RX);  
}

void LoRa_init() {
  SPI.begin(LoRa_SCK, LoRa_MISO, LoRa_MOSI, LoRa_SS);
  LoRa.setPins(LoRa_SS, LoRa_RST, LoRa_DIO0);
  
  if (!LoRa.begin(LoRa_Frequency_RX)) {
    Serial.println(LoRa_str_failed);
    LoRa_display(LoRa_str_failed,0,20);
    while (1);
  }

  LoRa_display(LoRa_str_ok,0,20);
  
  LoRa.setSpreadingFactor(LoRa_SpreadingFactor);
  LoRa.setSignalBandwidth(LoRa_SignalBandwidth);
  LoRa.setCodingRate4(LoRa_CodingRate4);
  LoRa.enableCrc();
  LoRa.setTxPower(LoRa_TxPower);
  delay(3000);  
}

void LoRa_display(String LoRa_str_display, int LoRa_i_X, int LoRa_i_Y) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,10);
  display.println(LoRa_str_startup);
  display.setCursor(LoRa_i_X,LoRa_i_Y+10);
  display.print(LoRa_str_display);
  display.setTextSize(1);
  display.setCursor(0,50);
  display.println(LoRa_str_call);
  display.display();  
}

void LoRa_init_display() {
  pinMode(LoRa_Oled_RST, OUTPUT);
  digitalWrite(LoRa_Oled_RST, LOW);
  delay(20);
  digitalWrite(LoRa_Oled_RST, HIGH);

  Wire.begin(LoRa_Oled_SDA, LoRa_Oled_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }
  display.clearDisplay();
  LoRa_display(LoRa_str_display_ok,0,20);
  delay(3000);   
}

String LoRaGetTime(time_t t)
{
	if(t == -1)
	{
		return String("00:00:00");
	}
	return String(LoRaNumberFormat(hour(t), 2) + "." + LoRaNumberFormat(minute(t), 2) + "." + LoRaNumberFormat(second(t), 2));
}

String LoRaNumberFormat(unsigned int number, unsigned int width)
{
	String result;
	String num(number);
	if(num.length() > width)
	{
		width = num.length();
	}
	for(unsigned int i = 0; i < width - num.length(); i++)
	{
		result.concat('0');
	}
	result.concat(num);
	return result;
}