#include <WiFi.h>
#include <EEPROM.h>

WiFiServer server(80);
const int EEPROM_SIZE = 96;

void clearPreferences() {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < EEPROM_SIZE; i++) EEPROM.write(i, 0);
  EEPROM.commit();
  Serial.println("WiFi credentials cleared.");
}

void saveCredentials(String ssid, String password) {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.writeString(0, ssid);
  EEPROM.writeString(32, password);
  EEPROM.commit();
}

bool readCredentials(String &ssid, String &password) {
  EEPROM.begin(EEPROM_SIZE);
  ssid = EEPROM.readString(0);
  password = EEPROM.readString(32);
  return ssid.length() > 0;
}

void startAPMode() {
  Serial.println("📡 Starting Setup Mode...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-Setup", "esp32pass");
  delay(1000);
  Serial.println("AP IP: " + WiFi.softAPIP().toString());
  server.begin();
}

void handleAPServer() {
  WiFiClient client = server.available();
  if (!client) return;

  while (client.connected() && !client.available()) delay(1);
  String req = client.readStringUntil('\r');
  client.readStringUntil('\n');

  if (req.startsWith("GET / ")) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println(R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
        <title>ESP32 Wi-Fi Setup</title>
        <style>
          body {
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            text-align: center;
            padding: 50px;
          }
          form {
            background: #fff;
            display: inline-block;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
          }
          input[type="text"], input[type="password"] {
            padding: 10px;
            margin: 10px;
            width: 80%;
            border: 1px solid #ccc;
            border-radius: 4px;
          }
          input[type="submit"] {
            padding: 10px 20px;
            background: #007bff;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
          }
          input[type="submit"]:hover {
            background: #0056b3;
          }
        </style>
      </head>
      <body>
        <h2>Configure Wi-Fi</h2>
        <form action="/save" method="GET">
          <input type="text" name="s" placeholder="SSID"><br>
          <input type="password" name="p" placeholder="Password"><br>
          <input type="submit" value="Save & Connect">
        </form>
      </body>
      </html>
    )rawliteral");
  }
  else if (req.startsWith("GET /save")) {
    int ssidIndex = req.indexOf("s=") + 2;
    int passIndex = req.indexOf("p=") + 2;
    String ssid = req.substring(ssidIndex, req.indexOf("&", ssidIndex));
    String password = req.substring(passIndex, req.indexOf(" ", passIndex));
    ssid.replace("+", " ");
    password.replace("+", " ");

    saveCredentials(ssid, password);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();

    client.println("<!DOCTYPE html><html><head><style>");
    client.println("body { font-family: Arial; text-align: center; padding: 40px; }");
    client.println("p { font-size: 18px; }");
    client.println("</style></head><body>");
    client.println("<h3>Attempting to connect to Wi-Fi...</h3>");

    WiFi.begin(ssid.c_str(), password.c_str());
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
      delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
      client.println("<p>Connected successfully!</p>");
      client.println("<p><b>IP Address:</b> " + WiFi.localIP().toString() + "</p>");
      client.println("<p>Rebooting in 5 seconds...</p>");
      client.println("</body></html>");
      client.stop();
      delay(5000);
      ESP.restart();
    } else {
      client.println("<p>Failed to connect. Please check your credentials.</p>");
      client.println("<a href='/'>Try Again</a>");
      client.println("</body></html>");
    }
  }

  client.stop();
}

bool connectToSavedWiFi() {
  String ssid, password;
  if (!readCredentials(ssid, password)) {
    Serial.println("No saved credentials.");
    return false;
  }

  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting to WiFi");

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Connected: " + WiFi.localIP().toString());
    return true;
  }

  Serial.println("\n❌ Failed to connect.");
  return false;
}

void handleWebServer() {
  // no-op for now; add lightweight handlers if needed
}