#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define BOT_TOKEN ""

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
LiquidCrystal_I2C lcd(0x27, 16, 2);
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

String name[2] = { "Krishna", "Gokul" };
String id[2] = { "4D 86 5A 37", "A3 B3 4D 91" };
String number[2] = {"1155647099","1309774037"};
float balance[2] = { 500, 500 };

int passanger[2] = { 0, 0 };
unsigned int startingMeterCount[2] = { 0, 0 };
unsigned int exitMeterCount[2] = { 0, 0 };
float distance[2] = { 0, 0 };
float cost[2] = { 0, 0 };

void setup(void) {
  Serial.begin(115200);
  Wire.begin();
  nfc.begin();
  lcd.init();
  lcd.clear();
  lcd.backlight();
  telegram_init();
}

void loop() {
  String tag = readNFC();
  if (tag != "") {
    int i;
    for (i = 0; i < 2; i++) {
      if (id[i] == tag) {
        passanger[i]++;
        break;
      }
    }
    if (passanger[i] == 1) {
      startingMeterCount[i] = getMeterCount();
      print(i);
    } else if (passanger[i] == 2) {
      exitMeterCount[i] = getMeterCount();
      distance[i] = (exitMeterCount[i] - startingMeterCount[i])/10.0;
      cost[i] = distance[i]*2;
      balance[i] -= cost[i];
      print(i);
      sendSMS(i);
      passanger[i] = 0;
    }
  }
}

void telegram_init(){
  lcd.setCursor(0,0);
  lcd.print("Waiting for WiFi");
  lcd.setCursor(0,1);
  lcd.print("connection .....");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setTrustAnchors(&cert);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  configTime(0, 0, "pool.ntp.org"); 
  time_t now = time(nullptr);
  while (now < 24 * 3600) {
    delay(100);
    now = time(nullptr);
  }
  delay(100);
  lcd.clear();
}

String readNFC() {
  String tagId = "";
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    tagId = tag.getUidString();
  }
  return tagId;
}

unsigned int getMeterCount() {
  Wire.requestFrom(8, 2);
  unsigned int value = (Wire.read() << 8) | Wire.read();
  return value;
}

void print(int &index) {
  if (passanger[index] == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Hello");
    lcd.setCursor(0, 1);
    lcd.print(name[index]);
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Starting Meter");
    lcd.setCursor(0, 1);
    lcd.print("Count : ");
    lcd.setCursor(8, 1);
    lcd.print(startingMeterCount[index]);
    delay(1000);
    lcd.clear();
  } else if (passanger[index] == 2) {
    lcd.setCursor(0, 0);
    lcd.print("Thank You");
    lcd.setCursor(0, 1);
    lcd.print(name[index]);
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Exit Meter");
    lcd.setCursor(0, 1);
    lcd.print("Count : ");
    lcd.setCursor(8, 1);
    lcd.print(exitMeterCount[index]);
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cost : ");
    lcd.setCursor(7, 0);
    lcd.print(cost[index]);
    lcd.setCursor(0, 1);
    lcd.print("Distance : ");
    lcd.setCursor(11, 1);
    lcd.print(distance[index]);
    lcd.setCursor(14, 1);
    lcd.print("Km");
    delay(1000);
    lcd.clear();
  }
}

void sendSMS(int &index){
  String message = "Rs. " + (String)cost[index] + " is deducted for a distance of " + (String)distance[index] + "Km.\nBalance : Rs. " + balance[index];
  bot.sendMessage(number[index],message, "");
}
