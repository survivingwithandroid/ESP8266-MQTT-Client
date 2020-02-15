#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <PubSubClient.h>

#define DHT_TYPE DHT11
#define DHT_PIN D5

char data[100];

IPAddress mqttServer(192,168,1,142);

const char *SSID = "your-wifi-ssid";
const char *PWD = "your_pwd";
long lastime = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Callback");
  Serial.println((char) payload[0]);
}

WiFiClient wifiCliient = WiFiClient();
PubSubClient mqttClient(mqttServer, 1883, callback, wifiCliient);
DHT dht(DHT_PIN, DHT_TYPE);



void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(SSID);
  
  WiFi.begin(SSID, PWD);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    // we can even make the ESP32 to sleep
  }

  Serial.print("Connected - ");
  //Serial.println(WiFi.localIP);
}

void reconnect() {
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
      Serial.println("Reconnecting to MQTT Broker..");
      String clientId = "ESP8266Client-";
      clientId += String(random(0xffff), HEX);
      
      if (mqttClient.connect(clientId.c_str())) {
        Serial.println("Connected.");
        // subscribe to topic
        mqttClient.subscribe("/esp8266/commands");
      }
  }
}

void setup() {
  Serial.begin(9600);
  connectToWiFi();
  dht.begin();
  Serial.println("DHT11 sensor....");
}

void loop() {

  if (!mqttClient.connected())
    reconnect();
  
  mqttClient.loop();


  // Publishing data to MQTT
  long now = millis();
  if(now - lastime > 10000) {
    Serial.println("Publishing data..");
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    sprintf(data, "%f", temp);
    Serial.println(data);
    mqttClient.publish("/esp8266/temperature", data);
    sprintf(data, "%f", hum);
    Serial.println(hum);
    mqttClient.publish("/esp8266/humidity", data);
    lastime = now;
  }

}