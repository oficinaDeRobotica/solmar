#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>

#ifdef ESP32
  #include <ESP32Servo.h>
  #include <WiFi.h>
#else
  #include <Servo.h>
  #include <ESP8266WiFi.h>
#endif

// mudar para o nome de seu wifi e a sua senha
const char *ssid = "esp";
const char *password = "12341234";
const int dns_port = 53;
const int http_port = 80;
const int ws_port = 1337;

// Motor A
#ifdef ESP32
  const int m1p1 = 25;
  const int m1p2 = 33;
  const int pwm1 = 14;
  const int servoPin = 12;
  const int freq = 30000;
  const int pwmChannel = 0;
  const int resolution = 8;
#else
  const int m1p1 = 14; // D5
  const int m1p2 = 12; // D6
  const int pwm1 = 13; // D7
  const int servoPin = 4;    // D2
#endif

// servo
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
int px, py, v = 0;

void moverRobo()
{

  v = 0;
  
  delay(20);

  if (py < 0)
  {
    digitalWrite(m1p1, HIGH);
    digitalWrite(m1p2, LOW);

    // converte o valor do joystick para o valor correto para a ponte HAumenta a velocidade do motor
    v = map(py, -10, -100, 150, 255);
    ;
  }
  else if (py > 0)
  {
    digitalWrite(m1p1, LOW);
    digitalWrite(m1p2, HIGH);

    // converte o valor do joystick para o valor correto para a ponte H
    v = map(py, 10, 100, 150, 255);
  }
  // Faz com que o motor fique parado caso o joystick esteja centralizado
  else
  {
    digitalWrite(m1p1, HIGH);
    digitalWrite(m1p2, HIGH);
    v = 0;
  }

  diff = map(px, -100, 100, 180, 0);
  servo.write(diff);

#ifdef ESP32
  // o esp 32 nao suporta o analogWrite, logo, funcao diferente
  ledcWrite(pwmChannel, v);
#else
  analogWrite(pwm1, v);
#endif
}

// Callback: recebendo mensagens websocket
void onWebSocketEvent(uint8_t client_num, WStype_t type, uint8_t *payload, size_t length)
{

  // descobrir qual o tipo de mensagem/evento
  switch (type)
  {

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
    strtokindex = strtok((char *)payload, " ");
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

void setup()
{
  pinMode(m1p1, OUTPUT);
  pinMode(m1p2, OUTPUT);
  pinMode(pwm1, OUTPUT);
  servo.attach(servoPin);

#ifdef ESP32
  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(pwm1, pwmChannel);
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

void loop()
{

  // Look for and handle WebSocket data
  webSocket.loop();
  moverRobo();
}
