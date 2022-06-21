#include <WiFi.h>
#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>

// Constants
//mude para o seu wifi e a sua senha
const char *ssid = "esp";
const char *password =  "12341234";
const int dns_port = 53;
const int http_port = 80;
const int ws_port = 1337;

// Motor A
const int motor1Pin1 = 25; 
const int motor1Pin2 = 33; 
const int enable1Pin = 14; 

// Motor B
const int motor2Pin1 = 27; 
const int motor2Pin2 = 26; 
const int enable2Pin = 12; 

const int freq = 30000;
const int pwmChannel = 0;
const int pwmChannel2 = 1;
const int resolution = 8;

// Globals
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(1337);
char msg_buf[10];
char *strtokindex;
int px, py, v1, v2 = 0;
float d = 0;

/***********************************************************
   Functions
*/

void moverRobo(){
  
  v1 = 0;
  v2 = 0;
  
  delay(20);
  
  if (py < 0) {
    
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, HIGH);
    digitalWrite(motor2Pin2, LOW);
    
    //Aumenta a velocidade do motor
    v1 = map(py, -10, -100, 150, 255);
    v2 = map(py, -10, -100, 150, 255);
  }
  else if (py > 0) {
    
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH);
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, HIGH);
    
    // Aumenta a velocidade do motor
    v1 = map(py, 10, 100, 143, 255);
    v2 = map(py, 10, 100, 143, 255);
  }
    //Faz com que os motores fiquem parados caso o joystick esteje centralizado
    else {
    v1 = 0;
    v2 = 0;
  }

  if (px > 10){
    d = map(px, 10, 100, 100, 80);
    d = d/100;
    v1 = v1 * d;
  }else if (px < -10){
    d = map(px, -10, -100, 100, 80);
    d = d/100;
    v2 = v2 * d;
  }

  ledcWrite(pwmChannel, v1);
  ledcWrite(pwmChannel2, v2);
}

// Callback: receiving any WebSocket message
void onWebSocketEvent(uint8_t client_num,
                      WStype_t type,
                      uint8_t * payload,
                      size_t length) {

  // Figure out the type of WebSocket event
  switch (type) {

    // Client has disconnected
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", client_num);
      break;

    // New client has connected
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(client_num);
        Serial.printf("[%u] Connection from ", client_num);
        Serial.println(ip.toString());
      }
      break;

    // Handle text messages from client
    case WStype_TEXT:
    {
      // Print out raw message
      //Serial.printf("%s\n", payload);
    strtokindex = strtok((char*) payload, " ");
    px = atoi(strtokindex);
    
    strtokindex = strtok(NULL, " ");
    py = atoi(strtokindex);
    
    Serial.print("velocidade motor 1: ");
    Serial.print(v1);
    Serial.print("  2: ");
    Serial.println(v2);

//    Serial.print("X: ");
//    Serial.print(px);
//    Serial.print("  Y: ");
//    Serial.println(py);
    }
    break;

    // For everything else: do nothing
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
     break;
  }
}

// Callback: send homepage
void onIndexRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                 "] HTTP GET request of " + request->url());
  request->send(SPIFFS, "/index.html", "text/html");
}

// Callback: send style sheet
void onCSSRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                 "] HTTP GET request of " + request->url());
  request->send(SPIFFS, "/style.css", "text/css");
}

void onJsRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                 "] HTTP GET request of " + request->url());
  request->send(SPIFFS, "/joy.js", "text/javascript");
}

// Callback: send 404 if requested file does not exist
void onPageNotFound(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                 "] HTTP GET request of " + request->url());
  request->send(404, "text/plain", "Not found");
}

/***********************************************************
   Main
*/

void setup() {

  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enable2Pin, OUTPUT);

  ledcSetup(pwmChannel, freq, resolution);
  ledcSetup(pwmChannel2, freq, resolution);
  
  ledcAttachPin(enable1Pin, pwmChannel);
  ledcAttachPin(enable2Pin, pwmChannel2);

  // Start Serial port
  Serial.begin(115200);

  // Make sure we can read the file system
  if ( !SPIFFS.begin()) {
    Serial.println("Error mounting SPIFFS");
    while (1);
  }   

  // Start access point
  WiFi.softAP(ssid, password);

  // Print our IP address
  Serial.println();
  Serial.println("AP running");
  Serial.print("My IP address: ");
  Serial.println(WiFi.softAPIP());

  // On HTTP request for root, provide index.html file
  server.on("/", HTTP_GET, onIndexRequest);

  // js request
  server.on("/joy.js", HTTP_GET, onJsRequest);

  // On HTTP request for style sheet, provide style.css
  server.on("/style.css", HTTP_GET, onCSSRequest);

  // Handle requests for pages that do not exist
  server.onNotFound(onPageNotFound);

  // Start web server
  server.begin();

  // Start WebSocket server and assign callback
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

}

void loop() {

  // Look for and handle WebSocket data
  webSocket.loop();
  moverRobo();
}
