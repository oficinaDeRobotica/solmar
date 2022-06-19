#include <Servo.h>

Servo leme;

//Configuração de pinos
#define lemePin 4
#define forwardPin1 5
#define reversePin1 6
#define enable1 7
#define forwardPin2 10
#define reversePin2 11
#define enable2 12

//Variáveis para receber os comandos do bluetooth
char commandChar;
char command[5];
int commandPointer = 0;

//Valor atual de comando do leme e motor, de 00 a 99
int lemeValue;
int motorValue;
//Valor da velocidade do motor
int velocidadeMotor = 0;
int angleLeme = 0;

void setup() {
  //Serial é o bluetooth, nos pinos 0 e 1.
  Serial.begin(9600);

  //Configura pinos como OUTPUT
  pinMode(lemePin, OUTPUT);
  pinMode(forwardPin1, OUTPUT);
  pinMode(reversePin1, OUTPUT);
  pinMode(forwardPin2, OUTPUT);
  pinMode(reversePin2, OUTPUT);  
  pinMode(enable1,OUTPUT);
  pinMode(enable2,OUTPUT);

  //Parar o motor na inicialização
  digitalWrite(forwardPin1,0);
  digitalWrite(reversePin1,0);
  digitalWrite(forwardPin2,0);
  digitalWrite(reversePin2,0);

  //Iniciar servo (para o zero)
  leme.attach(lemePin);
}

//Converte CHAR para INT (menos "0")
int char2int(char input) {
  return input - '0';
}

//Move o motor
void motorCommand(int motorValue) {
  //Se o comando for 50, centro do joystick, parar o motor  
  if (motorValue == 50) {
    digitalWrite(forwardPin1,0);
    digitalWrite(reversePin1,0);
    digitalWrite(forwardPin2,0);
    digitalWrite(reversePin2,0);  
    velocidadeMotor = 0;  
  }

  //Maior que 50, motor para frente  
  if (motorValue < 50) {
    digitalWrite(forwardPin1,HIGH);
    digitalWrite(forwardPin2,HIGH);
    digitalWrite(reversePin1,LOW);
    digitalWrite(reversePin2,LOW );
    velocidadeMotor = map(motorValue,50,0,150,255);
  }

  //Menor que 50, motor para frente
  if (motorValue > 50) {
    digitalWrite(reversePin1,LOW);
    digitalWrite(reversePin2,LOW);
    digitalWrite(forwardPin1,HIGH);
    digitalWrite(forwardPin2,HIGH);
    velocidadeMotor = map(motorValue,50,100,150,255);
  }
  Serial.print(velocidadeMotor);
  analogWrite(enable1,velocidadeMotor);
}

//Envia comandos para o leme
void lemeCommand(int lemeValue) {
  angleLeme = map(lemeValue,0,99,0,180); 
  leme.write(angleLeme);
  Serial.print(angleLeme);
}

//Loop!
void loop() {
  //Se há transmissão via bluetooth (na porta serial pinos 0 e 1)
  if (Serial.available()) {
    //ler o caracter enviado
    commandChar = Serial.read();

    //Remova o comentário abaixo para ter uma forma de debug...
    //Serial.println(commandChar);
    
    //Se o caracter for L então é o início da mensagem
    if (commandChar == 'L') {
      commandPointer = 0;      
    } else {
      commandPointer++;
    }

    if (commandPointer < 6) {
      //Armazena o comando em command (array)
      command[commandPointer] = commandChar;    
    } 
  }  

  //Se o ponteiro atinge 5 então o comando está completo (L00M00)
  //                                                      012345  <-  

  if (commandPointer >= 5) {
    //Remova o comentário abaixo para ter uma forma de debug...
    Serial.println(command);
    
    //Converte CHAR para INT considenrando que o primeiro char vale 10x
    lemeValue = 10*char2int(command[1]) + char2int(command[2]);

    //Converte CHAR para INT considenrando que o primeiro char vale 10x
    motorValue = 10*char2int(command[4]) + char2int(command[5]);

    //Volta o ponteiro para zero
    commandPointer=0;

    //Envia os dois comandos com os valores capturados
    motorCommand(motorValue);
    lemeCommand(lemeValue);
  }

  //Talvez seja necessário um DELAY aqui.  
}
