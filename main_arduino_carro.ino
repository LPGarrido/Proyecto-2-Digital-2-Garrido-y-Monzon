#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

const char* ssid = "ESP32";
const char* password = "91738520";

AsyncWebServer server(80);

String processor(const String& var) {
  Serial.println(var);

  return String();
}

void setup() {
  Serial.begin(115200);

  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  Serial.print("Setting AP (Access Point)…");
  //WiFi.setmode(WiFi.SOFTAP);
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();

  Serial.print("AP IP address: ");
  Serial.println(IP);

  setServerRoutes();

  // Initialize server
  server.begin();
  Serial.println("Server initialized");

  pinMode(D2, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D5, OUTPUT);

  Parar();
}

void loop() {

}

void Adelante() {
  //Dirección motor A
  digitalWrite (D2, HIGH);
  digitalWrite (D3, LOW);

  //Dirección motor B
  digitalWrite (D5, HIGH);
  digitalWrite (D6, LOW);
}

void Derecha() {
  //Dirección motor A
  digitalWrite (D2, LOW);
  digitalWrite (D3, LOW);

  //Dirección motor B
  digitalWrite (D5, HIGH);
  digitalWrite (D6, LOW);

}

void Izquierda() { // ya
  //Dirección motor A
  digitalWrite (D2, HIGH);
  digitalWrite (D3, LOW);

  //Dirección motor B
  digitalWrite (D5, LOW);
  digitalWrite (D6, LOW);

}

void Atras() { // ya
  //Dirección motor A
  digitalWrite (D2, LOW);
  digitalWrite (D3, HIGH);

  //Dirección motor B
  digitalWrite (D5, LOW);
  digitalWrite (D6, HIGH);
}

void Parar() {
  //Dirección motor A
  digitalWrite (D2, LOW);
  digitalWrite (D3, LOW);

  //Dirección motor B
  digitalWrite (D5, LOW);
  digitalWrite (D6, LOW);

}


void setServerRoutes() {
  // Serve html page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/index.html", String(), false, processor);
  });

  // Serve css
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/style.css", "text/css");
  });

  // Serve javascript file
  server.on("/index.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/index.js", "text/javascript");
  });

  // Serve joy js

  server.on("/joy.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/joy.min.js", "text/javascript");
  });


  //  forward
  server.on("/forward", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("forward");
    Adelante();
    request->send(200, "application/json", "{\"result\":true}");
  });

  //  stop
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("stop");
    Parar();
    request->send(200, "application/json", "{\"result\":true}");
  });

  //  reverse
  server.on("/reverse", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("reverse");
    Atras();
    request->send(200, "application/json", "{\"result\":true}");
  });

  //  right
  server.on("/right", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("right");
    Derecha();
    request->send(200, "application/json", "{\"result\":true}");
  });

  //  left
  server.on("/left", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("left");
    Izquierda();
    request->send(200, "application/json", "{\"result\":true}");
  });
}
