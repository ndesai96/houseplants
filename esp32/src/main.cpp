#include <Arduino.h>
#include "FileReader.h"
#include <I2CSoilMoistureSensor.h>
#include <I2CScanner.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <Wire.h>

WiFiManager wm;

char mqttServer[40]; // use ipconfig getifaddr en0 to find your local IP address
char houseplantName[40];
const char* mqttTopicPrefix = "houseplants/";
char mqttTopic[80];

// Interval to wait for light measurement to complete
const long waitForLightInterval = 3000; // 3 seconds

// Interval to wait between sensor reads
// Must be greater than waitForLightInterval
const long readInterval = 10000; // 10 seconds

uint8_t sensorAddress = 0x20;
I2CSoilMoistureSensor sensor(sensorAddress);

FileReader fileReader;

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

void setupWiFiManager() {
  WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", mqttServer, 40);
  WiFiManagerParameter custom_houseplant_name("houseplant", "Houseplant Name", houseplantName, 40);
  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_houseplant_name);

  wm.setConfigPortalTimeout(180); // 3 minutes

  if (!wm.autoConnect("HouseplantMonitorAP")) {
    Serial.println("Failed to connect and hit timeout");
    ESP.restart();
  }

  // If you get here you have connected to the WiFi
  wm.stopConfigPortal();

  Serial.println("Connected to WiFi: " + String(WiFi.SSID()));
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Read the custom parameters
  strncpy(mqttServer, custom_mqtt_server.getValue(), sizeof(mqttServer));
  strncpy(houseplantName, custom_houseplant_name.getValue(), sizeof(houseplantName));

  // Validate that the parameters are not empty
  if (strlen(mqttServer) == 0 || strlen(houseplantName) == 0) {
    Serial.println("MQTT Server or Topic is empty. Clearing WiFi settings and restarting.");
    Serial.println("Please re-configure with non-empty values.");
    
    wm.resetSettings();

    delay(3000);
    ESP.restart();
  }

  // Construct the MQTT topic
  snprintf(mqttTopic, sizeof(mqttTopic), "%s%s", mqttTopicPrefix, houseplantName);

  Serial.print("MQTT Server: ");
  Serial.println(mqttServer);
  Serial.print("Plant Name: ");
  Serial.println(houseplantName);
  Serial.print("MQTT Topic: ");
  Serial.println(mqttTopic);
}

void setCerts() {
  if (!fileReader.begin()) {
    Serial.println("Failed to initialize file system");
    while (1);
  }

  const char* caCert = fileReader.readFile("/ca.crt");
  if (caCert == nullptr) {
    Serial.println("Failed to read ca.crt");
    while (1);
  }

  const char* sensorCert = fileReader.readFile("/sensor.crt");
  if (sensorCert == nullptr) {
    Serial.println("Failed to read sensor.crt");
    while (1);
  }

  const char* sensorKey = fileReader.readFile("/sensor.key");
  if (sensorKey == nullptr) {
    Serial.println("Failed to read sensor.key");
    while (1);
  }

  wifiClient.setCACert(caCert);
  wifiClient.setCertificate(sensorCert);
  wifiClient.setPrivateKey(sensorKey);
}

void connectToMQTT() {
  mqttClient.setServer(mqttServer, 8883);  

  while (!mqttClient.connected()) {
    Serial.printf("Connecting to MQTT broker %s...", mqttServer);
    if (mqttClient.connect(houseplantName)) {
      Serial.println("success");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" trying again in 2 seconds");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setupWiFiManager();

  Wire.begin();
  delay(100);
  sensor.begin();

  if (!sensor.validateAddress()) {
    Serial.print("Sensor not found at address 0x");
    Serial.println(sensorAddress, HEX);
    while (1);
  } else {
    Serial.print("Current Sensor I2C address: 0x");
    Serial.println(sensor.getAddress(), HEX);
  }

  Serial.print("Sensor Firmware version: ");
  Serial.println(sensor.getVersion(), HEX);

  setCerts();
}

bool publishData(unsigned int moisture, float temperature, unsigned int light) {
  String payload = "{";
  payload += "\"moisture\":" + String(moisture) + ",";
  payload += "\"temperature\":" + String(temperature, 2) + ",";
  payload += "\"light\":" + String(light);
  payload += "}";
  Serial.println("Payload: " + payload);
  return mqttClient.publish(mqttTopic, payload.c_str());
}

enum State {
  WAITING_FOR_INTERVAL,
  WAITING_FOR_LIGHT,
  READING_SENSORS,
  PUBLISHING_DATA,
};

State currentState = WAITING_FOR_INTERVAL;

unsigned int moisture;
float temperature;
unsigned int light;

unsigned long lastSensorRead = 0;
unsigned long startedMeasuringLight = 0;

void loop() {
  if (!mqttClient.connected()) {
    connectToMQTT();
  }
  mqttClient.loop();

  unsigned long currentTime = millis();

  switch (currentState) {
    case WAITING_FOR_INTERVAL: {
      if (currentTime - lastSensorRead >= readInterval - waitForLightInterval) {
        startedMeasuringLight = currentTime;
        sensor.measureLight();
        currentState = WAITING_FOR_LIGHT;
      }
      break;
    }

    case WAITING_FOR_LIGHT: {
      if (currentTime - startedMeasuringLight >= waitForLightInterval) {
        currentState = READING_SENSORS;
      }
      break;
    }

    case READING_SENSORS: {
      moisture = sensor.getMoisture();
      temperature = sensor.getTemperature() / 10.0;
      light = sensor.getLight(false);
      currentState = PUBLISHING_DATA;
      lastSensorRead = currentTime;
      break;
    }

    case PUBLISHING_DATA: {
      if (!publishData(moisture, temperature, light)) {
        Serial.print("Failed to publish data to ");
        Serial.println(mqttTopic);
      }
      currentState = WAITING_FOR_INTERVAL;
      break;
    }
  }
}