#include "config.h"
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

String LoRa_incomig_Data;

Adafruit_SSD1306 display(Lora_Screen_Width, LoRa_Screen_Height, &Wire, LoRa_Oled_RST);

long l_Timer_Bake = 0;
long l_Timer_Bake_Send = ((1000*60)*LoRa_Timer_Bake);

void LoRa_send(String LoRa_str_Data, int LoRa_i_Header);
void LoRa_init_display();
void LoRa_init();
void LoRa_display(String LoRa_str_display, int LoRa_i_X, int LoRa_i_Y);

void setup() {
  LoRa_init_display();
  LoRa_init();

  Serial.begin(LoRa_Serial_Baudrate);

  String loRa_str_Bake = (String)LoRa_str_call + ">" + String(LoRa_str_Dest) + ":!" + (String)LoRa_str_Lat + (String)LoRa_str_Overlay + (String)LoRa_str_Lon + (String)LoRa_str_Symbol+(String)LoRa_str_Comment;
  LoRa_send(loRa_str_Bake, 1);
  LoRa_display("Send Bake",0,20);
}

void loop() {
  if (millis() > l_Timer_Bake_Send + l_Timer_Bake ) {
    l_Timer_Bake = millis();
    
    String loRa_str_Bake = (String)LoRa_str_call + ">" + String(LoRa_str_Dest) + ":!" + (String)LoRa_str_Lat + (String)LoRa_str_Overlay + (String)LoRa_str_Lon + (String)LoRa_str_Symbol+(String)LoRa_str_Comment;
    LoRa_send(loRa_str_Bake, 1);
    LoRa_display("Send Bake",0,20);
  }
  
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    LoRa_display("Received packet",0,20);

    while (LoRa.available()) {
      LoRa_incomig_Data = LoRa.readString();

      int i_str_Call = LoRa_incomig_Data.indexOf('>');
      String strSource = LoRa_incomig_Data.substring(3, i_str_Call);

      int i_str_Path = LoRa_incomig_Data.indexOf(":");
      String strPath = LoRa_incomig_Data.substring(i_str_Call+1, i_str_Path);
      String strDest = LoRa_incomig_Data.substring(i_str_Call+1, i_str_Path);
      String strPaths = "";

      int i_str_Paths = strPath.indexOf(",");
      if (i_str_Paths != -1 ) {
        strDest = "";
        strDest = LoRa_incomig_Data.substring(i_str_Call+1, i_str_Call + 1 +  i_str_Paths);
        strPaths = strPath.substring(i_str_Paths+1, strPath.length());
        strPaths = "," + strPaths;
      }

      int i_str_repeated = strPath.indexOf("*");
      int i_str_is_wide = strPath.indexOf("WIDE1-1");
      int i_str_is_dest = strDest.indexOf("-1");

      if (i_str_repeated = -1) {
        if (i_str_is_wide != -1 || i_str_is_dest != -1) {
          String str_Display_incomming = strSource + " repeated!";
          LoRa_display(str_Display_incomming,0,20);
          LoRa_incomig_Data.replace(strPath, strDest + "," + (String)LoRa_str_call + "*" + strPaths);
          LoRa_send(LoRa_incomig_Data,0);
        }
      }
    }
  } 
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
  display.setCursor(0,0);
  display.println(LoRa_str_startup);
  display.setCursor(LoRa_i_X,LoRa_i_Y);
  display.print(LoRa_str_display);
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