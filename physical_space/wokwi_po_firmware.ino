#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>

// --- Configuration ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "";
const char* mqtt_user = "";
const char* mqtt_pass = "";
const int mqtt_port = 8883;

DHTesp dht;
Servo myServo;
WiFiClientSecure espClient;
PubSubClient client(espClient);

const int PIN_TRIG = 5;
const int PIN_ECHO = 18;

// Callback for Strong Entanglement (Actuation)
void callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<200> cmd;
  deserializeJson(cmd, payload, length);
  if (cmd.containsKey("servo_angle")) {
    int angle = cmd["servo_angle"];
    myServo.write(angle);
    Serial.printf("📥 Actuation: Servo moved to %d\n", angle);
  }
}

void setup() {
  Serial.begin(115200);
  dht.setup(15, DHTesp::DHT22);
  myServo.attach(13);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  espClient.setInsecure(); // Skip certificate check for simulation
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Reflection: Mirroring status
  TempAndHumidity dhtData = dht.getTempAndHumidity();
  digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(10); digitalWrite(PIN_TRIG, LOW);
  float distance = pulseIn(PIN_ECHO, HIGH) / 58.0;

  StaticJsonDocument<300> doc;
  doc["sensor_group"] = "factory_zone_A";
  doc["thermal"]["temp"] = dhtData.temperature;
  doc["thermal"]["hum"] = dhtData.humidity;
  doc["proximity"]["distance_cm"] = distance;
  doc["status"] = "OPERATIONAL";

  char buffer[300];
  serializeJson(doc, buffer);
  client.publish("factory/zone_A/po_status", buffer);
  
  delay(5000); 
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("Wokwi_PO_Client", mqtt_user, mqtt_pass)) {
      client.subscribe("factory/zone_A/actuator_cmds");
    } else {
      delay(5000);
    }
  }
}