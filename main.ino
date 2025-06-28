#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <RBDdimmer.h>
#include "secrets.h"  // enthält: ssid, password, mqtt_server, mqtt_user, mqtt_pass

#define outputPin 13     // GPIO13 (DIM)
#define zerocross 14     // GPIO14 (ZC)

WiFiClient espClient;
PubSubClient client(espClient);
dimmerLamp dimmer(outputPin, zerocross);

// WLAN verbinden
void setup_wifi() {
    Serial.print("Verbinde mit WLAN: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWLAN verbunden!");
}

void publishDimmerState() {
  char msg[5];
  sprintf(msg, "%d", dimmer.getPower());
  client.publish("esp12/dimmer/state", msg, true);  // 'true' = retained
}

void callback(char* topic, byte* payload, unsigned int length) {
  char msg[10];
  if (length >= sizeof(msg)) return;
  strncpy(msg, (char*)payload, length);
  msg[length] = '\0';


  Serial.print("MQTT empfangen: ");
  Serial.println(msg);

  if (strcmp(msg, "ON") == 0) {
    dimmer.setState(ON);
  } else if (strcmp(msg, "OFF") == 0) {
    dimmer.setState(OFF);
  } else {
    int value = atoi(msg);
    value = constrain(value, 0, 100);
    dimmer.setPower(value);
    dimmer.setState(ON);
  }

  publishDimmerState();  // ← Status zurückmelden!
}


// Verbindung zum MQTT-Broker aufbauen
void reconnect() {
  while (!client.connected()) {
    Serial.print("Verbindung zu MQTT-Broker... ");
    if (client.connect("ESP12_Dimmer", mqtt_user, mqtt_pass)) {
      Serial.println("Erfolgreich verbunden!");
      client.subscribe("esp12/dimmer/set");

      client.publish("esp12/test", "Hallo von ESP12", true);
      const char* discovery_topic = "homeassistant/light/boiler_dimmer/config";
      const char* discovery_payload =
      "{\"name\":\"Boiler\",\"uniq_id\":\"boiler_1\",\"command_topic\":\"esp12/dimmer/set\",\"state_topic\":\"esp12/dimmer/state\",\"brightness_command_topic\":\"esp12/dimmer/set\",\"brightness_scale\":100,\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"platform\":\"mqtt\"}";


      delay(250);  // kleine Pause, damit MQTT bereit ist
      bool ok = client.publish(discovery_topic, discovery_payload, true);
      Serial.print("MQTT Discovery senden: ");
      Serial.println(ok ? "✔️ erfolgreich" : "❌ fehlgeschlagen");

    } else {
      Serial.print("Fehlgeschlagen, rc=");
      Serial.print(client.state());
      Serial.println(" – erneuter Versuch in 5 Sekunden...");
      delay(5000);
    }
  }
}

void setup() {
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    dimmer.begin(NORMAL_MODE, ON);
    dimmer.setPower(0);  // Startleistung 0 %
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    // Optionale Statusausgabe
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 5000) {
        Serial.print("Aktuelle Leistung: ");
        Serial.print(dimmer.getPower());
        Serial.println(" %");
        lastLog = millis();
    }
}
