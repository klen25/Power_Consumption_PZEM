#include <WiFi.h>
#include "ThingSpeak.h" // Library Thingspeak
#include "PZEM004Tv30.h" // Library PZEM004Tv30
#include <stdio.h>
#include <string.h>

//PZEM004Tv30 pzem(16,17);
PZEM004Tv30 pzem(Serial2);
#define Detik 1000
#define TOKirim 60
#define MaxClient 2
#define TOWait 3

uint8_t detik = 0, Client = 0, Wait = 0;
bool WaitDataClient, NextDataClient, GetDataClient, TimeToSend, Kirim = false;

char ssid[] = "Pohon Belimbing";
char password[] = "klentang25";
WiFiClient  client;
unsigned long myChannelNumber = 2033395;
char WriteKey[] = "NDH8XH7J2I65IOWU";
unsigned long prevmillis = 0;
float voltage, current, power;
float current1, voltage1, power1;
float current2, voltage2, power2;
float KWH, kwh;
uint8_t WattCount = 0;

// Fungsi untuk membaca tegangan dari PZEM004Tv30
float cekvolt() {
  float voltage = pzem.voltage();
  if (voltage >= 0.0) {
  }
  else {
    voltage = 0.0;
  }
  return voltage;  
}

// Fungsi untuk membaca arus dari PZEM004Tv30
float cekcurrent() {
  float current = pzem.current();
  if (current >= 0.0) {
  }
  else {
    current = 0.0;
  }
  return current;  
}

// Fungsi untuk membaca daya dari PZEM004Tv30
float cekpower() {
  float power = pzem.power();
  if (power >= 0.0) {
  }
  else {
    power = 0.0;
  }
  return power;  
}

// Format Data dari Client = #CL_ID|Tegangan|Arus|Daya*
void CekData(String Input) {
  uint8_t i = 0, k = 0, l = 0;
  if((Input[0] == '#') and (Input[1] == 'C') and (Input[2] == 'L') and (Input[Input.length()-1] == '*')) {
    if((int(InputRF[3]-48) == Client)) {
      WaitDataClient = false;
      NextDataClient = true;

      for(i = 3; i <= InputRF.length()-2; i++) {
        if(Input[i] == '|') {
          k=k+1;
          dt[k] ="";
        }
        else {
          dt[k] = dt[k] + Input[i];
        }
      }
      if(CL == 1) {
        voltage1 = (float)atof(dt[1]);
        current1 = (float)atof(dt[2]);
        power1 = (float)atof(dt[3]);
      }
      if(CL == 2) {
        voltage2 = (float)atof(dt[1]);
        current2 = (float)atof(dt[2]);
        power2 = (float)atof(dt[3]);
      }
    }
  }
}

void setup() {
  Serial.begin(9600);
//  Serial2.begin(9600,SERIAL_8N1,16,17);
  WiFi.mode(WIFI_STA);
  uint8_t x = 0;
  do {
    if(WifiStatus()) {
      break;
    }
    else {
      x++;
      delay(1000);
    }
  }
  while(x < 5);
  ThingSpeak.begin(client);
}

void loop() {
//  if(WaitDataClient) {
//    if (Serial2.available()) {
//      String Inp = Serial2.readString();
//    }
//    CekData(Inp);
//  }

  if(millis() - prevmillis >= Detik) {
    prevmillis = millis();
    detik++;
    voltage = cekvolt();
    current = cekcurrent();
    power = cekpower();
    kwh += power * ((TOKirim / 1000.0) / 60);
    KWH = kwh / WattCount;

    Serial.printf("V: %0.2f|I: %0.2f|P: %0.2f\n", voltage, current, power);
    if(detik > TOKirim) { detik = 0; GetDataClient = true; }

    if(GetDataClient) {
      if(NextDataClient) Client++;
//      if(Client <= MaxClient && Wait > TOWait) { Serial2.printf("#REQ|%d*", Client); Wait = 0; }
      Wait++;
      WaitDataClient = true;
    }
    
    if(TimeToSend) {
      if(WifiStatus()) {
        TimeToSend = false;
        Kirim = true;
      }
      else {
        TimeToSend = false;
      }
    }
  }

  if(Kirim) {
    SendToThinspeak();
    Kirim = false;
  }
}

void SendToThinspeak() {
  ThingSpeak.setField(1, voltage);
  ThingSpeak.setField(2, current);
  ThingSpeak.setField(3, power);
  ThingSpeak.setField(4, voltage1);
  ThingSpeak.setField(5, current1);
  ThingSpeak.setField(6, power1);
  ThingSpeak.setField(7, voltage2);
  ThingSpeak.setField(8, current2);
  ThingSpeak.setField(9, power2);

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
//  if(x == 200) { Serial.println("Channel update successful."); }
//  else { Serial.println("Problem updating channel. HTTP error code " + String(x)); }
}

bool WifiStatus() {
/* // Jika harus dipastikan sampai terkoneksi dulu
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print("."); //delay(1000);
    }
    Serial.println("\nConnected.");
  }
 */
/* // Jika hanya cek saja sekali, lalu kembali ke loop
 */
  WiFi.begin(ssid, password);
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
    return false;
  }
  else {
    Serial.println("Connected to WiFi");
    return true;
  }
}
