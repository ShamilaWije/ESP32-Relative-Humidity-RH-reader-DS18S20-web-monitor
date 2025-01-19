#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include <AsyncElegantOTA.h>
#include <math.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TM1637.h>

const char* ssid = "shamila";
const char* password = "12345678";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

int outputGPIOs[1] = {2};

#define buttonPin1 33 
#define buttonPin2 25 
#define buttonPin3 26 
#define buttonPin4 27 
#define relay 12 
#define WIFI_CONNECT_TIMEOUT 15 
// Pin 3 - > DIO
// Pin 2 - > CLK
TM1637 tm(17, 16);

// For DS1820
OneWire oneWire1(15);  // pin 2
OneWire oneWire2(13);  // pin 3

DallasTemperature sensors1(&oneWire1);
DallasTemperature sensors2(&oneWire2);

float T_dry;
float T_wet;
float RH;


bool G01 = 0;
bool Y01 = 0;
bool Y02 = 0;
bool R01 = 0;

bool rs = 1;
bool rc = 1;
bool wifiConnected = 0;
bool sIP = 0;

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  unsigned long startMillis = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startMillis) < (WIFI_CONNECT_TIMEOUT * 1000)) {
    Serial.print('.');
    tm.display("NET ");
    delay(1000);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(WiFi.localIP());
    wifiConnected = true;
    sIP = 1;
  } else {
    Serial.println("WiFi connection failed or timed out. Running without WiFi.");
    wifiConnected = false;
  }
}

void displayLastOctets() {
  IPAddress localIP = WiFi.localIP();
  int part3 = localIP[2];
  int part4 = localIP[3];
  tm.display("    "); // Clear display
  tm.display(String(part3));
  delay(500);
}

void displayLastOctets2 () {
  IPAddress localIP = WiFi.localIP();
  int part3 = localIP[2];
  int part4 = localIP[3];
  tm.display("    "); // Clear display
  tm.display(String(part4));
  delay(500);
  
}

const float rhSetpointStep = 5.0; // Step size for adjusting setpoint
const float rhSetpointMin = 0.0;   // Minimum setpoint value
const float rhSetpointMax = 100.0;
float rhSetpoint = 60;

String getOutputStates(){
  JSONVar myArray;
  for (int i =0; i<1; i++){
    myArray["gpios"][i]["output"] = String(outputGPIOs[i]);
    myArray["gpios"][i]["state"] = String(digitalRead(outputGPIOs[i]));
  }
  myArray["T_dry"] = String(T_dry, 2);
  myArray["T_wet"] = String(T_wet, 2);
  myArray["RH"] = String(RH, 2);
  myArray["rhSetpoint"] = String(rhSetpoint, 2);

  String jsonString = JSON.stringify(myArray);
  return jsonString;
}

void notifyClients(String state) {
  ws.textAll(state);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "states") == 0) {
      notifyClients(getOutputStates());
    }
    else{
      int gpio = atoi((char*)data);
      if (gpio == 2){
        digitalWrite(gpio, !digitalRead(gpio));
        notifyClients(getOutputStates());
      }else{
        String BC = (char*)data;
        if (BC == "BR1"){
          R01=1;
        }else if (BC == "BY1"){
          Y01=1;
        }else if (BC == "BY2"){
          Y02=1;
        }else if (BC == "BG1"){
          G01=1;
        }
      }      
    }
  }
}


void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

void pG1() {
  if (digitalRead(buttonPin2)==0){
    initWiFi();
    if (wifiConnected) {
    initWebSocket();
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html", "text/html", false);
    });
    server.serveStatic("/", SPIFFS, "/");
    AsyncElegantOTA.begin(&server);
    }
  }else{
    G01 = 1;
  }
}

void pY2() {
  if (digitalRead(buttonPin3)==0){
    if (wifiConnected) {
      displayLastOctets();
    }
  }else{
    Y02 = 1;
  }
}

void pY1() {
  Y01 = 1;
}

void pR1() {
  if (digitalRead(buttonPin3)==0){
    if (wifiConnected) {
      displayLastOctets2();
    }
  }else{
    R01 = 1;
  }
  
}

void showTemp(){
  tm.display("    ");
  tm.display(" TP ");
  delay(1000);
  tm.display(T_dry);
  delay(1500);
  tm.display(" rH ");
  delay(400);
}

float calculateRelativeHumidity(float T_dry, float T_wet)
{
  const float es_dry = 6.112 * exp((17.502 * T_dry) / (T_dry + 240.97));
  const float es_wet = 6.112 * exp((17.502 * T_wet / (T_wet + 240.97)));
  const float ea = es_wet - 0.67 * (1 + 0.001 * T_wet) * (T_dry - T_wet);
  const float RH = (ea / es_dry) * 100.0;
  return RH;
}

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

IPAddress Actual_IP;
// definitions of your desired intranet created by the ESP32
IPAddress PageIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ip;

void setup(){
  Serial.begin(115200);
  tm.begin();
  tm.setBrightness(4);
  for (int i =0; i<1; i++){
    pinMode(outputGPIOs[i], OUTPUT);
  }
  initSPIFFS();
  initWiFi();
  if (wifiConnected) {
    initWebSocket();
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html", "text/html", false);
    });
    server.serveStatic("/", SPIFFS, "/");
    AsyncElegantOTA.begin(&server);
  }
  
  sensors1.begin();
  sensors2.begin();

  sensors1.setResolution(12);
  sensors2.setResolution(12);

  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);
  pinMode(buttonPin4, INPUT_PULLUP);
  pinMode(relay, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(buttonPin1), pG1, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPin2), pY2, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPin3), pY1, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPin4), pR1, FALLING);

  server.begin();
}

void loop() {
  if (G01 == 1){
    showTemp();
    G01 =0;   
  }
  if (R01 == 1){
    rs =!rs;
    if (rs == 0){
      tm.display("OFF ");
    }else{
      tm.display(" ON ");
    }    
    R01 = 0;    
  }
  if (Y01 == 1){
    rhSetpoint = min(rhSetpoint + rhSetpointStep, rhSetpointMax);
    tm.display(rhSetpoint);
    Y01 = 0; 
  }
  if (Y02 == 1){
     rhSetpoint = max(rhSetpoint - rhSetpointStep, rhSetpointMin);
     tm.display(rhSetpoint);
     Y02 = 0; 
  }
  if (sIP == 1){
    IPAddress localIP = WiFi.localIP();
    int part1 = localIP[0];
    int part2 = localIP[1];
    int part3 = localIP[2];
    int part4 = localIP[3];
    tm.display("    "); 
    tm.display(String(part1));
    delay(1000);
    tm.display("    ");
    tm.display(String(part2));
    delay(1000);
    tm.display("    "); 
    tm.display(String(part3));
    delay(1000);
    tm.display("    ");
    tm.display(String(part4));
    delay(1000);
    sIP = 0;
  }
  sensors1.requestTemperatures();
  sensors2.requestTemperatures();
  float sensor1_temp = sensors1.getTempCByIndex(0);
  float sensor2_temp = sensors2.getTempCByIndex(0);
  
  if (sensor1_temp > sensor2_temp) {
    T_dry = sensor1_temp;
    T_wet = sensor2_temp;
  } else {
    T_dry = sensor2_temp;
    T_wet = sensor1_temp;
  }

  RH = calculateRelativeHumidity(T_dry, T_wet);

  tm.display(RH);
  if (RH > rhSetpoint && rs==1){
    digitalWrite(relay,1);
  }else if(RH < rhSetpoint && rs==1){
    digitalWrite(relay,0);
  }else if(RH < rhSetpoint && rs==0){
    digitalWrite(relay,0);
  }else if (RH > rhSetpoint && rs==0){
    digitalWrite(relay,0);
  }
  
  JSONVar AA1;
  AA1["T_dry"] = String(T_dry, 2);
  AA1["T_wet"] = String(T_wet, 2);
  AA1["RH"] = String(RH, 2);
  AA1["rhSetpoint"] = String(rhSetpoint, 2);
  AA1["relayState"] = String(rs); 

    // Convert the JSON object to a string
  String jsonStr = JSON.stringify(AA1);
  if (wifiConnected) {
    notifyClients(jsonStr);
    ws.cleanupClients();
  }
  delay(10);
}