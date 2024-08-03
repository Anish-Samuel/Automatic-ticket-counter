#include <Wire.h>
#define interruptPin 2

unsigned int count = 0;

void blink() {
  count++;
}

void requestEvent() {
  byte arr[2];
  arr[0] = count>>8;
  arr[1] = count;
  Wire.write(arr,2);
}

void setup() {
  Serial.begin(115200);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, RISING);
  Wire.begin(8);
  Wire.onRequest(requestEvent);
}

void loop() {
}
