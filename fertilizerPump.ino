#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino_JSON.h>

const char* ssid = "SSID";
const char* password = "SSID_PASSWORD";

const String server = "AZUREFUNCTION_URL";

unsigned long last_time = 0;
unsigned long timer_delay = 15000;

int pumpDuration = 3000;
int pumpPin = 5;

WiFiClient wifiClient;


void setup() {
  pinMode(pumpPin, OUTPUT);
  pumpOff();
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WIFI…");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("After 5 seconds the first reading will be displayed");
  delay(1000);
}

void loop() {
  if ((millis() - last_time) > timer_delay) {
    if (WiFi.status() == WL_CONNECTED) {
      boolean execute = getExecutePump();
      if (execute) {
        runPump();
      }
    }
    else {
      Serial.println("WiFi is Disconnected!");
    }
    last_time = millis();
  }
}

void runPump() {
  Serial.println("Running pump.");
  pumpOn();
  delay(pumpDuration);
  pumpOff();
  postExecute();
  Serial.println("Pump done. Wating 60 seconds to resume.");

  for (int i = 1; i < 61; i++) {
    Serial.println(i);
    delay(1000);

  }
}

void pumpOff() {
  digitalWrite(pumpPin, HIGH);
}

void pumpOn() {
  digitalWrite(pumpPin, LOW);
}

boolean getExecutePump() {
  String payload = httpGETRequest( server + "fertilizerExecute?pump=1");

  Serial.println(payload);
  JSONVar executePump = JSON.parse(payload);

  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(executePump) == "undefined") {
    Serial.println("Parsing input failed!");
    return false;
  }

  Serial.print("JSON object = ");
  Serial.println(executePump);

  boolean execute = executePump["execute"];

  pumpDuration = executePump["duration"];
  return execute;
}

void postExecute() {
  String serverName = server + "fertilizer";
  String httpRequestData = "{\"pump\":\"1\"}";
  httpPOSTRequest(serverName, httpRequestData);
}


String httpGETRequest(String serverName) {
  WiFiClient client;
  HTTPClient http;

  // Your IP address with path or Domain name with URL path
  http.begin(client, serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    payload = http.getString();
  }

  Serial.print("HTTP Response code is: ");
  Serial.println(httpResponseCode);
  http.end();

  return payload;
}

void httpPOSTRequest(String serverName, String httpRequestData) {
  WiFiClient client;
  HTTPClient http;

  http.begin(client, serverName);

  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(httpRequestData);

  Serial.print("HTTP Response code is: ");
  Serial.println(httpResponseCode);
  http.end();
}
