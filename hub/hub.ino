/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp-now-many-to-one-esp32/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/
#include <ESP32_NOW.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#define ESPNOW_WIFI_CHANNEL 6

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  JsonDocument doc;
  deserializeJson(doc, incomingData, len);
  int id = doc["i"];
  float humidity = doc["h"];
  Serial.print("h");
  Serial.print(id);
  Serial.print(":");
  Serial.println(humidity);
}
 
void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);
  
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  while (!WiFi.STA.started()) {
    delay(100);
  }
  
  Serial.println("Initialized ESP-NOW");
  Serial.println(WiFi.macAddress());

  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}
 
void loop() {
  delay(100);  
}
