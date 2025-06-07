#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <FS.h>  // File system
#include <SPIFFS.h>

WiFiClientSecure secureClient;  // ✅ only declared once
PubSubClient client(secureClient);

const char* mqtt_server = "192.168.1.17";  // your broker IP

void loadCACert() {
  File file = SPIFFS.open("/certs/ca.crt");
  if (!file || file.isDirectory()) {
    Serial.println("❌ Failed to open CA cert file");
    return;
  }

  size_t size = file.size();
  std::unique_ptr<char[]> buf(new char[size + 1]);
  file.readBytes(buf.get(), size);
  buf[size] = '\0';

  secureClient.setCACert(buf.get());
  Serial.println("✅ CA certificate loaded");
}

void callback(char* topic, byte* message, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)message[i];
  }
  Serial.printf("Message arrived [%s]: %s\n", topic, msg.c_str());

  if (msg == "open") {
    // driveMotor(200);
  } else if (msg == "close") {
    // driveMotor(-200);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT TLS connection...");
    if (client.connect("esp32-client", "esp32user", "yourpassword")) {
      Serial.println("connected");
      client.subscribe("door/control");
    } else {
      Serial.printf("failed, rc=%d\n", client.state());
      delay(5000);
    }
  }
}

void setupMessanger() {
  loadCACert();                             // Load cert from SPIFFS
  client.setServer(mqtt_server, 8883);      // Secure MQTT port
  client.setCallback(callback);
}

void handleMessanger() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}