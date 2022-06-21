#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>

//m1p1 = motor 1 pin 1
//pwm1 = pino enable da ponte H para o motor 1

#ifdef ESP32
  #include <WiFi.h>
  // Motor A
  const int m1p1 = 25; 
  const int m1p2 = 33; 
  const int pwm1 = 14; 
  // Motor B
  const int m2p1 = 27; 
  const int m2p2 = 26; 
  const int pwm2 = 12;
  //variaveis pro pwm do esp32, mais complicado
  const int freq = 30000;
  const int pwmChannel = 0;
  const int pwmChannel2 = 1;
  const int resolution = 8;
#else
  #include <ESP8266WiFi.h>
  //motor A
  const int m1p1 = 5; //D1 
  const int m1p2 = 4; //D2
  const int pwm1 = 0; //D3
  //motor B
  const int m2p1 = 2;  //D4
  const int m2p2 = 14; //D5
  const int pwm2 = 12; //D6
#endif

//mudar para o nome de seu wifi e a sua senha
const char *ssid = "esp";
const char *password =  "12341234";
const int dns_port = 53;
const int http_port = 80;
const int ws_port = 1337;

int px, py, v1, v2 = 0;
float d = 0;

// tempo atual
unsigned long currentTime = millis();
// tempo passado
unsigned long previousTime = 0; 
// definir tempo de timeout (exemplo: 2000ms = 2s)
const long timeoutTime = 2000;

// variaveis globais
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(1337);
char msg_buf[10];
char *strtokindex;

void moverRobo(){
  
  v1 = v2 = 0;
  
  delay(20);
  
  if (py < 0) {
    
    digitalWrite(m1p1, HIGH);
    digitalWrite(m1p2, LOW);
    digitalWrite(m2p1, HIGH);
    digitalWrite(m2p2, LOW);
    
    //mapeia o joystick pro alcance do pwm do motor
    v1 = v2 = map(py, -10, -100, 150, 255);
  }
  else if (py > 0) {
    
    digitalWrite(m1p1, LOW);
    digitalWrite(m1p2, HIGH);
    digitalWrite(m2p1, LOW);
    digitalWrite(m2p2, HIGH);
    
    //mapeia o joystick pro alcance do pwm do motor
    v1 = v2 = map(py, 10, 100, 143, 255);
  }
    //Faz com que os motores fiquem parados caso o joystick esteje centralizado
    else {
    v1 = v2 = 0;
  }

  //faz o calculo da direção diferencial e reduz a velocidade de um motor específico
  if (px > 10){
    d = map(px, 10, 100, 100, 80);
    d = d/100;
    v1 = v1 * d;
  }else if (px < -10){
    d = map(px, -10, -100, 100, 80);
    d = d/100;
    v2 = v2 * d;
  }

  #ifdef ESP32
    //o esp 32 nao suporta o analogWrite, logo, funcao diferente
    ledcWrite(pwmChannel, v1);
    ledcWrite(pwmChannel2, v2);
  #else
    analogWrite(pwm1, v1);
    analogWrite(pwm2, v2);
  #endif
  
}

// Callback: recebendo mensagens websocket
void onWebSocketEvent(uint8_t client_num, WStype_t type, uint8_t * payload, size_t length) {

  // descobrir qual o tipo de mensagem/evento
  switch (type) {

    // Cliente desconectou
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", client_num);
      break;

    // Novo cliente conectou
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(client_num);
        Serial.printf("[%u] Connection from ", client_num);
        Serial.println(ip.toString());
      }
      break;

    case WStype_TEXT:
    {
    strtokindex = strtok((char*) payload, " ");
    px = atoi(strtokindex);
    
    strtokindex = strtok(NULL, " ");
    py = atoi(strtokindex);
    
    /* somente para debug
    Serial.print("X: ");
    Serial.print(px);
    Serial.print("  Y: ");
    Serial.println(py);
    */
    }
    break;

    // para o resto: nao fazer nada
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

void setup() {
  pinMode(m1p1, OUTPUT);
  pinMode(m1p2, OUTPUT);
  pinMode(pwm1, OUTPUT);
  pinMode(m2p1, OUTPUT);
  pinMode(m2p2, OUTPUT);
  pinMode(pwm2, OUTPUT);

  #ifdef ESP32
    //configuracao do pwm do esp, mais complicado
    ledcSetup(pwmChannel, freq, resolution);
    ledcSetup(pwmChannel2, freq, resolution);
    ledcAttachPin(pwm1, pwmChannel);
    ledcAttachPin(pwm2, pwmChannel2);
  #endif

  // inicia porta serial
  Serial.begin(115200);
  // inicia ponto de acesso
  WiFi.softAP(ssid, password);
  // printa o ip no serial terminal
  Serial.println("meu ip: ");
  Serial.println(WiFi.softAPIP());
  // inicia servidor web
  server.begin();
  // inicia websocket
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void loop() {
  webSocket.loop(); //faz a comunicação com o app
  moverRobo(); //move o robo com os dados recebidos
}
