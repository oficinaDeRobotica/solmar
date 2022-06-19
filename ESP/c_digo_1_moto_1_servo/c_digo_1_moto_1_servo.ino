#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <Servo.h>
#ifdef ESP32
  #include <Wifi.h>
#else
  #include <ESP8266WiFi.h>
#endif

//mudar para o nome de seu wifi e a sua senha
const char *ssid = "esp";
const char *password =  "12341234";
const int dns_port = 53;
const int http_port = 80;
const int ws_port = 1337;

// Motor A
#ifdef ESP32
  const int motor1Pin1 = 25; 
  const int motor1Pin2 = 33; 
  const int enable1Pin = 14;
  const int servoPin = 12;
#else
  const int motor1Pin1 = 15; 
  const int motor1Pin2 = 12; 
  const int enable1Pin = 13;
  const int servoPin = 4;
#endif

//servo
Servo servo;
int diff;

// tempo atual
unsigned long currentTime = millis();
// tempo passado
unsigned long previousTime = 0; 
// definir tempo de timeout (exemplo: 2000ms = 2s)
const long timeoutTime = 2000;

// Globals
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(1337);
char msg_buf[10];
char *strtokindex;
int pot_x, pot_y, velocidademotor = 0;

/***********************************************************
   Functions
*/

void moverRobo(){
  
  velocidademotor = 0;;
  delay(20);
  
  if (pot_y < 0) {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    
    // converte o valor do joystick para o valor correto para a ponte HAumenta a velocidade do motor
    velocidademotor = map(pot_y, -10, -100, 150, 255);;
  }
  else if (pot_y > 0) {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH);
    
    // converte o valor do joystick para o valor correto para a ponte H
    velocidademotor = map(pot_y, 10, 100, 150, 255);
  }
    //Faz com que o motor fique parado caso o joystick esteja centralizado
    else {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, HIGH);
    velocidademotor = 0;
  }
  
  diff = map(pot_x,-100, 100, 0, 180);
  servo.write(diff);
  analogWrite(enable1Pin, velocidademotor);
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

void setup() {
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);
  servo.attach(servoPin);

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
  Serial.println("My IP address: ");
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
