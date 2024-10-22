/*
    ESP-NOW Broadcast Master
    Lucas Saavedra Vaz - 2024

    This sketch demonstrates how to broadcast messages to all devices within the ESP-NOW network.
    This example is intended to be used with the ESP-NOW Broadcast Slave example.

    The master device will broadcast a message every 5 seconds to all devices within the network.
    This will be done using by registering a peer object with the broadcast address.

    The slave devices will receive the broadcasted messages and print them to the Serial Monitor.
*/

#include <ArduinoJson.h>
#include <DHT22.h>
#include "ESP32_NOW.h"
#include "WiFi.h"
#include <esp_mac.h>  // For the MAC2STR and MACSTR macros

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#define ESPNOW_WIFI_CHANNEL 6

#define CHILD_ID 1

void data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

/* Global Variables */

uint32_t msg_count = 0;

DHT22 dht22(25); 

/* Main */

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  // Initialize the Wi-Fi module
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
  while (!WiFi.STA.started()) {
    delay(100);
  }

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_send_cb(data_sent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = ESPNOW_WIFI_CHANNEL;  
  peerInfo.encrypt = false;
         
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    Serial.println("Rebooting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }

  Serial.print("Child sensor ");
  Serial.println(CHILD_ID);
  Serial.println("Wi-Fi parameters:");
  Serial.println("  Mode: STA");
  Serial.println("  MAC Address: " + WiFi.macAddress());
  Serial.printf("  Channel: %d\n", ESPNOW_WIFI_CHANNEL);

  Serial.println(dht22.debug());

  Serial.println("Setup complete. Broadcasting messages every 5 seconds.");
}

void loop() {
  float t = dht22.getTemperature();
  float h = dht22.getHumidity();

  JsonDocument doc;
  doc["m"] = WiFi.macAddress();
  doc["i"] = CHILD_ID;

  if (dht22.getLastError() != dht22.OK) {
    auto e = dht22.getLastError();
    Serial.print("last error :");
    Serial.println(e);

    doc["r"] = "error";
    doc["e"] = e;
  } else {
    doc["r"] = "ok";
    doc["h"] = h;
    doc["t"] = t;
  }

  // Broadcast a message to all devices within the network
  char message[255];
  
  size_t msg_len = serializeJson(doc, message, sizeof message);

  Serial.write(message, msg_len);
  Serial.println();

  esp_err_t outcome = esp_now_send(broadcastAddress, (uint8_t *) &message, msg_len);

  if (outcome != ESP_OK) {
    Serial.println("Failed to broadcast message");
  }

  delay(1000);
}
