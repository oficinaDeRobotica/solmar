#include <WiFi.h>
#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ESP32Servo.h>

// Constants
//mudar para o nome de seu wifi e a sua senha
const char *ssid = "esp";
const char *password =  "12341234";
const int dns_port = 53;
const int http_port = 80;
const int ws_port = 1337;
// Motor A
const int motor1Pin1 = 25; 
const int motor1Pin2 = 33; 
const int enable1Pin = 14; 

//servo
Servo servo;
static const int servoPin = 12;

// Motor B
//const int motor2Pin1 = 27; 
//const int motor2Pin2 = 26; 
//const int enable2Pin = 18; 

const int freq = 30000;
const int pwmChannel = 2;
//const int pwmChannel2 = 1;
const int resolution = 8;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// Globals
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(1337);
char msg_buf[10];
char *strtokindex;
int pot_x = 0;
int pot_y = 0;
float diff = 0;
int velocidademotor1, velocidademotor2 = 0;

/***********************************************************
   Functions
*/

void moverRobo(){
  
  velocidademotor1 = 0;
  velocidademotor2 = 0;
  diff = 0;
  
  delay(20);
  
  if (pot_y < 0) {
    
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    //digitalWrite(motor2Pin1, HIGH);
    //digitalWrite(motor2Pin2, LOW);
    
    //Aumenta a velocidade do motor
    velocidademotor1 = map(pot_y, -10, -100, 150, 255);
    //velocidademotor2 = map(pot_y, -10, -100, 150, 255);
  }
  else if (pot_y > 0) {
    
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH);
    //digitalWrite(motor2Pin1, LOW);
    //digitalWrite(motor2Pin2, HIGH);
    
    // Aumenta a velocidade do motor
    velocidademotor1 = map(pot_y, 10, 100, 143, 255);
    //velocidademotor2 = map(pot_y, 10, 100, 143, 255);
  }
    //Faz com que os motores fiquem parados caso o joystick esteje centralizado
    else {
    velocidademotor1 = 0;
    //velocidademotor2 = 0;
  }

/*
  if (pot_x > 10){
    diff = map(pot_x, 10, 100, 100, 80);
    diff = diff/100;
    velocidademotor1 = velocidademotor1 * diff;
  }else if (pot_x < -10){
    diff = map(pot_x, -10, -100, 100, 80);
    diff = diff/100;
    velocidademotor2 = velocidademotor2 * diff;
  }
  */
  Serial.print("velocidade motor 1: ");
  Serial.print(velocidademotor1);

  //Serial.print("  2: ");
  //Serial.println(velocidademotor2);

  diff = map(pot_x,-100, 100, 0, 180);
  servo.write(diff);
  ledcWrite(pwmChannel, velocidademotor1);
  //ledcWrite(pwmChannel2, velocidademotor2);
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
    pot_x = atoi(strtokindex);
    
    strtokindex = strtok(NULL, " ");
    pot_y = atoi(strtokindex);
    
    Serial.print("X: ");
    Serial.print(pot_x);
    Serial.print("  Y: ");
    Serial.println(pot_y);
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
  request->send(SPIFFS, "/updated.html", "text/html");
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
  //pinMode(motor2Pin1, OUTPUT);
  //pinMode(motor2Pin2, OUTPUT);
  //pinMode(enable2Pin, OUTPUT);

  ledcSetup(pwmChannel, freq, resolution);
  //ledcSetup(pwmChannel2, freq, resolution);
  
  ledcAttachPin(enable1Pin, pwmChannel);
  //ledcAttachPin(enable2Pin, pwmChannel2);

  // Start Serial port
  Serial.begin(115200);

  // Make sure we can read the file system
  if ( !SPIFFS.begin()) {
    Serial.println("Error mounting SPIFFS");
    while (1);
  }   

  // Allow allocation of all timers for servo library
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  // Set servo PWM frequency to 50Hz
  servo.setPeriodHertz(50);
  
  // Attach to servo and define minimum and maximum positions
  // Modify as required
  servo.attach(servoPin,500, 2400);

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
