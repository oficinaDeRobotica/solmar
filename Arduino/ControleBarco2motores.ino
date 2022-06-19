//Configuração de pinos
#define motor1forward 5
#define motor1reverse 6
#define enable1 7
#define motor2forward 10
#define motor2reverse 11
#define enable2 12

//Variáveis para receber os comandos do bluetooth
char commandChar;
char command[5];
int commandPointer = 0;

//Valor atual de comando do leme e motor, de 00 a 99
int motor1Value;
int motor2Value;
//Valor da velocidade
int velocidadeMotor1;
int velocidadeMotor2;
void setup() {
  //Serial é o bluetooth, nos pinos 0 e 1.
  Serial.begin(9600);

  //Configura pinos como OUTPUT
  pinMode(motor1forward, OUTPUT);
  pinMode(motor1reverse, OUTPUT);
  pinMode(motor2forward, OUTPUT);
  pinMode(motor2reverse, OUTPUT);
  pinMode(enable1,OUTPUT);
  pinMode(enable2,OUTPUT);

  //Parar o motor na inicialização
  digitalWrite(motor1forward,LOW);
  digitalWrite(motor1reverse,LOW);
  digitalWrite(motor2forward,LOW);
  digitalWrite(motor2reverse,LOW);
  digitalWrite(enable1,0);
  digitalWrite(enable2,0);
}

//Converte CHAR para INT (menos "0")
int char2int(char input) {
  return input - '0';
}

//Move o motor
void motor1Command(int motor1Value) {
  //Se o comando for 50, centro do joystick, parar o motor  
  if (motor1Value == 50) {
    digitalWrite(motor1forward,LOW);
    digitalWrite(motor1reverse,LOW);
  }
  //Menor que 50, motor para trás
  else if (motor1Value < 50) {
    digitalWrite(motor1forward,LOW);
    digitalWrite(motor1reverse, HIGH);
    velocidadeMotor1 = map(motor1Value,50,0,150,255);
  }
  //Maior que 50, motor para frente
  else if (motor1Value > 50) {
    digitalWrite(motor1reverse,HIGH);
    digitalWrite(motor1forward,LOW);
    velocidadeMotor1 = map(motor1Value,50,100,150,255);
  }
  Serial.print(velocidadeMotor1);
  analogWrite(enable1,velocidadeMotor1);
}

//Move o segundo Motor
void motor2Command(int motor2Value) {
  //Se o comando for 50, centro do joystick, parar o motor  
  if (motor2Value == 50) {
    digitalWrite(motor2forward,LOW);
    digitalWrite(motor2reverse,LOW);
  }
  //Menor que 50, motor para trás
  else if (motor2Value < 50) {
    digitalWrite(motor2forward,LOW);
    digitalWrite(motor2reverse, HIGH);
    velocidadeMotor2 = map(motor2Value,50,0,150,255);
  }
  //Maior que 50, motor para frente
  else if (motor2Value > 50) {
    digitalWrite(motor2reverse,HIGH);
    digitalWrite(motor2forward,LOW);
    velocidadeMotor2 = map(motor2Value,50,100,150,255);
  }
  Serial.print(velocidadeMotor2);
  analogWrite(enable2,velocidadeMotor2);
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
    motor2Value = 10*char2int(command[1]) + char2int(command[2]);

    //Converte CHAR para INT considenrando que o primeiro char vale 10x
    motor1Value = 10*char2int(command[4]) + char2int(command[5]);

    //Volta o ponteiro para zero
    commandPointer=0;

    //Envia os dois comandos com os valores capturados
    motor1Command(motor1Value);
    motor2Command(motor2Value);
  }

  //Talvez seja necessário um DELAY aqui.  
}
