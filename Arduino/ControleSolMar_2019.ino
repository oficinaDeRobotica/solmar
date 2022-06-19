
#include <Servo.h>

Servo leme;

//Configuração de pinos
#define lemePin 5
#define pin1Motor1 11
#define pin2Motor1 6
#define Enable1 7

//Variáveis para receber os comandos do bluetooth
char commandChar;
char command[5];
int commandPointer = 0;

//Valor atual de comando do leme e motor, de 00 a 99
int lemeValue;
int motorVal = 0;
//Valor da velocidade do motor
int velocidadeMotor = 0;
int angleLeme = 0;
void setup() {
  //Serial é o bluetooth, nos pinos 0 e 1.
  Serial.begin(9600);

  //Configura pinos como OUTPUT
  pinMode(lemePin, OUTPUT);
  pinMode(pin1Motor1, OUTPUT);
  pinMode(pin2Motor1, OUTPUT);
  pinMode(Enable1,OUTPUT);

  //Parar o motor na inicialização
  digitalWrite(pin1Motor1,0);
  digitalWrite(pin2Motor1,0);
  digitalWrite(Enable1,0);

  //Iniciar servo (para o zero)
  leme.attach(lemePin);
}

//Converte CHAR para INT (menos "0")
int char2int(char input) {
  return input - '0';
}

//Move o motor
void motorCommand(int motorVal) {
  //Se o comando for 50, centro do joystick, parar o motor  
  if (motorVal == 50) {
    digitalWrite(pin1Motor1,0);
    digitalWrite(pin2Motor1,0);
    velocidadeMotor = 0;
  }

  //Número positivo, motor para frente  
  else if (motorVal>50) {
    digitalWrite(pin1Motor1,HIGH);
    digitalWrite(pin2Motor1,LOW);
    velocidadeMotor = map(motorVal,50,100,150,255);
  }

  //Número negativo, motor para trás
  else if(motorVal<50) {
    digitalWrite(pin1Motor1,LOW);
    digitalWrite(pin2Motor1,HIGH);
    velocidadeMotor = map(motorVal,50,0,150,255);
  }
  Serial.print(velocidadeMotor);
  analogWrite(Enable1,velocidadeMotor);
  
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
    //Serial.println(command);
    
    //Converte CHAR para INT considenrando que o primeiro char vale 10x
    lemeValue = 10*char2int(command[1]) + char2int(command[2]);

    //Converte CHAR para INT considenrando que o primeiro char vale 10x
    motorVal = 10*char2int(command[4]) + char2int(command[5]);

    //Volta o ponteiro para zero
    commandPointer=0;

    //Envia os dois comandos com os valores capturados
    motorCommand(motorVal);
    lemeCommand(lemeValue);
  }
  //Talvez seja necessário um DELAY aqui.  
}
