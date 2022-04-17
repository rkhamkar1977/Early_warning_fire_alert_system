#include <heltec.h>
#include <WiFi.h>

const char* ssid = "Terasic_RFS";
const char* pswd = "1234567890";
const char* host = "192.168.4.1";
const char* fire_url = "/SRDURDKFIR?";
const char* fire_resp_url = "/fire_resp";

String str = "";
RTC_DATA_ATTR unsigned int cycle = 0; 
RTC_DATA_ATTR unsigned int THRESHOLD = 400;
RTC_DATA_ATTR const int cycles_per_reset = 330;
int MODE = 0;
bool is_master = true;
const int NodeID = 132;
String packet = "";
String rssi = "";
String packSize = "";
int TIME_TO_SLEEP = 10;
int uS_TO_S_FACTOR = 1000000;
const int adc_pin = 36;
int adc_val = 0;
int window = 100;
int nod = NULL;
int sens_dat[2] = {NULL,NULL};

void print_sleep_message() {
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0 , 15 , "GOING TO SLEEP!");
  Heltec.display->drawString(0 , 30 , String(analogRead(adc_pin))+" "+String(THRESHOLD));
  Heltec.display->display();
  delay(2000);
}

void print_scanning_message() {
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0 , 15 , "SCANNING FOR SIG");
  Heltec.display->drawString(0 , 30 , String(analogRead(adc_pin))+" "+String(THRESHOLD));
  Heltec.display->display();
}

void LoraData() {
  if (MODE == 2) {
    Heltec.display->clear();
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(0 , 15 , "Rcvd FIR AlRT:" + packet);
    Heltec.display->drawStringMaxWidth(0 , 26 , 128, "AlRT NGHBR!");
    Heltec.display->drawString(0, 0, rssi);  
    Heltec.display->display();
  } 
  if (MODE == 1) {
    Heltec.display->clear();
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(0 , 15 , "UPDTING FIR THR TO:");
    Heltec.display->drawStringMaxWidth(0 , 26 , 128, packet);
    Heltec.display->drawString(0, 0, rssi);  
    Heltec.display->display();
  }
}

void calibrateThreshold() {
  int cnt = 0;
  int sum = 0;
  int adc_val = 0;
  while (cnt < 20) {
    adc_val = analogRead(adc_pin);
    sum += adc_val;
    cnt++;
    Serial.println("Analog signal:"+String(adc_val)+" Count:"+String(cnt)+" Sum:"+String(sum));//Serial
    delay(500);
  }
  Serial.println("Sum:"+String(sum));//Serial
  sum = sum/20;
  THRESHOLD = sum - 40;
  Serial.println("Sum:"+String(sum)+" "+String(THRESHOLD));//Serial
}

void parsePac() {
  int i = 0;
  int len = packet.length();
  const char *p = packet.c_str(); 
  int ptr = 0;
  while (ptr < len) {
    if (p[ptr] == '?') {
      i++  ;
    }
    ptr++;
  }
  switch (i){
  case 1:
    sscanf(p,"%d?%d", &nod, &sens_dat[0]);
    break;
  case 2:
    sscanf(p,"%d?%d?%d", &nod, &sens_dat[0], &sens_dat[1]);
    break;
  default:
    sscanf(p,"%d",&nod);
    break;
  }
}

int onRecv(int packetSize) {
//  packet = "";
String  header = "";
  packSize = String(packetSize,DEC);
  for (int i = 0; i < packetSize; i++) { 
    if (i > 3) 
      packet += (char) LoRa.read();
    else
      header += (char) LoRa.read();
  }
  if (header == "FIR ") {MODE = 2;}
  if (header == "LIM ") {MODE = 1;}
  rssi = "RSSI " + String(LoRa.packetRssi(),DEC); 
  LoraData();
}

void fireAlert(int node, int sens1, int sens2) {
if (sens2 == NULL) {
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0 , 15 , "FIRE! Node:"+String(node));
  Heltec.display->drawString(0 , 30 , String(sens1));
  Heltec.display->display();
  LoRa.beginPacket();
  LoRa.beginPacket();
  LoRa.setTxPower(20,RF_PACONFIG_PASELECT_PABOOST);
  LoRa.print("FIR "+String(node)+"?"+String(sens1));
  Serial.println("FIR "+String(node)+"?"+String(sens1));//Serial
  LoRa.endPacket();
} else {
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0 , 15 , "FIRE! Node:"+String(nod));
  Heltec.display->drawString(0 , 30 , String(sens1)+ " " +String(sens2));
  Heltec.display->display();
  LoRa.beginPacket();
  LoRa.beginPacket();
  LoRa.setTxPower(20,RF_PACONFIG_PASELECT_PABOOST);
  LoRa.print("FIR "+String(node)+"?"+String(sens1)+ "?" + String(sens2));
  Serial.println("FIR "+String(node)+"?"+String(sens1)+"?" + String(sens2));//Serial
  LoRa.endPacket();
}
}

int fireAlertFPGA(int node, int sens1, int sens2) {
  WiFi.begin(ssid, pswd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host,httpPort)) {
    return 0;
  }
  str = String("GET ") + fire_url + String(node) + String("?") + String(sens1) + String("?") + String(sens2) + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n";
  client.print(str);
  return 1;
}

void updateTh(int new_lim) {
  LoRa.beginPacket();
  LoRa.beginPacket();
  LoRa.setTxPower(20,RF_PACONFIG_PASELECT_PABOOST);
  LoRa.print("LIM "+String(new_lim));
  LoRa.endPacket();
}

void init_board() {
  Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, 915E6 /**/);
  Heltec.display->init();
  Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_10);
  //logo();
  delay(1500);
  Heltec.display->clear();
  
  Heltec.display->drawString(0, 0, "Board Initial success!");
  Heltec.display->display();
  delay(1000);
}



// the setup routine runs once when starts up

void setup(){
  // Initialize the Heltec ESP32 object
  init_board();
  Serial.begin(115200);//Serial
  if (cycle%cycles_per_reset == 0) {calibrateThreshold();}
  cycle++;
  Serial.println("Cycle:"+String(cycle)+" Threshold:"+String(THRESHOLD)+" Cycles per reset:"+String(cycles_per_reset));//Serial
  adc_val = analogRead(adc_pin);
  if (adc_val < THRESHOLD) {
    while (1) {
      fireAlert(NodeID, adc_val, NULL);
      delay(1000);
    }
  }
  int counter = 0;
  print_scanning_message();
  LoRa.receive();
  while (counter < window) {
      adc_val = analogRead(adc_pin);
      if (adc_val < THRESHOLD) {
        while (1) {
          fireAlert(NodeID, adc_val, NULL);
          delay(1000);
        }
      }
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
        onRecv(packetSize);
        parsePac();
        Serial.println("Recieved packet from Node:"+String(nod)+" sensor data:"+String(sens_dat[0])+" "+String(sens_dat[1]));//Serial
        break;
      }
      delay(100);
      counter++;
  }
  if (MODE == 2) {
    if(is_master) {
      if (sens_dat[1] == NULL) {sens_dat[1] = analogRead(adc_pin);}
      fireAlertFPGA(nod,sens_dat[0],sens_dat[1]);
    } else {
      while (1) {
        if (sens_dat[1] == NULL) {sens_dat[1] = analogRead(adc_pin);}
        fireAlert(nod, sens_dat[0], sens_dat[1]);
        delay(1000);
      }
    }
  }
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  LoRa.end();
  LoRa.sleep();
  print_sleep_message();
  esp_deep_sleep_start();
}

// the loop routine runs over and over again forever
void loop() {
}
