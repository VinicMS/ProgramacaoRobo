/* ========== BIBLIOTECAS ========== */
#include "PS2X_lib.h" //biblioteca de conexão com o receptor do controle de ps2
#include <stdio.h> //biblioteca de entrada e saída
#include <Servo.h> //biblioteca para o brushless (ele atua como um servo motor)

/* ========== FUNÇÕES ========== */
void botoes();

/* ========== DEFINIÇÕES ========== */
/**********************
* seta os pinos conectados ao controle do PS2
*   - coluna 1: original 
*   - coluna 2: pinos setados
* substitua o numero pelo numero do pino que ira utilizar
**********************/

#define PS2_DAT         8   // DATA
#define PS2_CMD         7   // COMMAND
#define PS2_SEL         6   // ATTENTION
#define PS2_CLK         5   // CLOCK

/**********************
* seleçao do modo de controle PS2:
*   - pressures = leitura analogica de botoes de pressao 
*   - rumble    = motor rumbling
* descomente 1 das linhas para cada modo de seleçao
**********************/

#define pressures   true // habilita os botoes de direçao com valores de 0->255
//#define pressures   false  // desabilita os botoes de direçao fixando o valor em 0
#define rumble      true // percebi que nesse modo o botao X quando pressionado faz o controle vibrar
//#define rumble      false

/* ========== DEFINIÇÕES DE PORTAS ========== */

Servo motorArma;

//Pino de controle do motor
int pino_motor = 2;

/* Ponte H */
int IN1 = 12;  // entrada 1 da ponte H
int IN2 = 11;  // entrada 2 da ponte H
int IN3 = 10;  // entrada 3 da ponte H
int IN4 =  9;  // entrada 4 da ponte H

PS2X ps2x; // cria um objeto da classe PS2
// Neste momento, a biblioteca NÃO suporta a conexão de controles ​"a quente", o que significa
// você deve sempre reiniciar seu Arduino depois de conectar o controle,
// ou ligue config_gamepad (pinos) novamente depois de conectar o controle.
// PROBLEMA RESOLVIDO POR LORENZO COM A VARIÁVEL resetControle

int error = 0;
byte type = 0;
byte vibrate = 0;

int resetControle = 0; // resolve o problema citado acima

int valor = 0;//valor de rotação para o motor brushless
int valorms = 700;
int contArma = 0;//controla o estado da arma: LIGADA ou DESLIGADA

/* ========== CONFIGURAÇÃO ========== */
void setup()
{  
  // seta os pinos da ponte H
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  digitalWrite(IN2, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  
  Serial.begin(9600);

  motorArma.attach(pino_motor);//define a porta digital do pino de entrada do brushless.

  Serial.println("Aguardando 5 segundos ...");
  
  delay(500);  // atraso adicional para fornecer o módulo ps2 sem fio algum tempo para iniciar, antes de configurá-lo, e para o esc estabelecer a conexao entre o arduino e o motor brushless
  
  //==========|| TESTES DE CONEXAO DO CONTROLE ||=============================================================================================================================================//
  
  //Configurar pinos e outras configuraçoes: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  
  if(error == 0)
  {
    Serial.print("Controle encontrado, configurado com sucesso! ");
    
    Serial.print("pressures = ");
    
    if (pressures)
      Serial.println("true ");    
    else
      Serial.println("false");
    
    Serial.print("rumble = ");
    
    if (rumble)
      Serial.println("true)");    
    else
    Serial.println("false");
    
    Serial.println("Experimentar todos os botoes, X vibrara o controle, mais rapido quanto mais forte vc pressionar;");
    Serial.println("Segurar L1 ou R1 ira imprimir os valores analogicos dos botoes '1'.");
    Serial.println("Nota: Go to www.billporter.info for updates and to report bugs.");
  }  
  else if(error == 1)
  Serial.println("Controle nao encontrado, checar as conexoes, olhe o arquivo readme.txt para habilitar o debug. visit www.billporter.info for troubleshooting tips");
  
  else if(error == 2)
  Serial.println("Controle encontrado mas nao aceita comandos. Olhe o arquivo readme.txt para habilitar o debug. Visit www.billporter.info for troubleshooting tips");
  
  else if(error == 3)
  Serial.println("Controle recusando entrar no modo Pressures, modo pode nao ser suportado. ");
  
  type = ps2x.readType(); 
  
  // -- Registradores de configuração do Timer2 --
  // POR FAVOR NÃO MEXA AQUI ARIEL
  TCCR2A = 0x00;   //Timer operando em modo normal
  TCCR2B = 0x07;   //Prescaler 1:1024
  TCNT2  = 100;    //10 ms overflow again
  TIMSK2 = 0x01;   //Habilita interrupção do Timer2
  
  //==========|| FIM TESTES DE CONEXAO DO CONTROLE ||=============================================================================================================================================//
}

/* ========== LOOP ========== */
void loop() 
{
  /* Voce deve ler o Gamepad para obter novos valores e setar os valores de vibraçao
  ps2x.read_gamepad(small motor on/off, larger motor strenght from 0-255)
  Se voce nao habilitar o rumble, use o ps2x.read_gamepad(); sem valores
  Voce deveria chama-la pelo menos uma vez por segundo.
  */
  // PRO FAVOR NÃO MEXA AQUI ARIEL
  while(resetControle==0)
  {
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
    resetControle++; 
  }
  
  // freio motor na ponte H
  digitalWrite(IN2, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);  
  
  if(error == 1) //sai do loop se o controle nao for encontrado
  {
    return;
  }
  
  //DualShock Controller
  ps2x.read_gamepad(false, vibrate); //faz o controle vibrar     //read controller and set large motor to spin at 'vibrate' speed
  
  botoes();
  
  delay(100);
}

/* ========== BOTÕES ========== */

void botoes()
{
  //BOTÕES L_R
  if (ps2x.NewButtonState())         //sera VERDADEIRO se qualquer botao mudar de estado(on to off, or off to on)
  {
    if(ps2x.Button(PSB_L3))
    {
      Serial.println("L3 pressionado.");
    }
    
    if(ps2x.Button(PSB_R2))
    {
      int contRotacao = 0;
      /*

      PROBLEMA ATUAL: O motor inicia no valor definido(170), mas quando R2 é clicado nvoamente, a rotação para, e não retorna novamente.
      Ideias: 
        * Circuito pode estar perdendo o contato.
        * Ele não está entendendo que o valor após o "zero" é um novo comando de rotação.
      
      */

      /*

      Lista das velocidades testadas:
      90: parou
      100: rodou mais devagar
      150: rodou no talo
      180: não rodou
      
      */
      
      contArma++;//incrementa a cada vez que clica no R2
      
      Serial.println("R2 pressionado.");
      
      if(contArma%2 == 0)
      {
        valor = 90;//angulação        
        motorArma.write(valor);
        Serial.print("Valor para o motor: ");
        Serial.println(valor);
      }
      else
      {
        valor = 171;//angulação
        motorArma.write(valor);
        Serial.print("Valor para o motor: ");
        Serial.println(valor);
      }
    }
  }
  
  if((ps2x.Analog(PSS_RX)>= 120 && ps2x.Analog(PSS_RX)<= 130) && (ps2x.Analog(PSS_LY)>= 120 && ps2x.Analog(PSS_LY)<= 130))
  {
    digitalWrite(IN2,LOW);
    digitalWrite(IN4,LOW);
    digitalWrite(IN1,LOW);
    digitalWrite(IN3,LOW);
    //Serial.println("PARADO");
  }
  
  else
  {
    if(ps2x.Analog(PSS_LY)> 140 && ps2x.Analog(PSS_LY)<=255)//148
    {
      digitalWrite(IN1,HIGH);
      digitalWrite(IN2,LOW);
      digitalWrite(IN3,HIGH);
      digitalWrite(IN4,LOW);
      //Serial.println(ps2x.Analog(PSS_LY));
      //Serial.println("TRAS");
    }
    
    else if(ps2x.Analog(PSS_LY) < 120 && ps2x.Analog(PSS_LY)>=0)//148
    {
      digitalWrite(IN1,LOW);
      digitalWrite(IN2,HIGH);
      digitalWrite(IN3,LOW);
      digitalWrite(IN4,HIGH);      
      //Serial.println(ps2x.Analog(PSS_LY));
      //Serial.println("FRENTE");
    }  
    if(ps2x.Analog(PSS_RX)> 140 && ps2x.Analog(PSS_RX)<=255)//148
    {
      
      
      
      digitalWrite(IN1,LOW);
      digitalWrite(IN2,HIGH);
      digitalWrite(IN3,HIGH);
      digitalWrite(IN4,LOW);
      //Serial.println(ps2x.Analog(PSS_LY));
      //Serial.println("DIREITA");
    }
    
    else if(ps2x.Analog(PSS_RX)< 120 && ps2x.Analog(PSS_RX)>=0)//148
    {      
      digitalWrite(IN1,HIGH);
      digitalWrite(IN2,LOW);
      digitalWrite(IN3,LOW);
      digitalWrite(IN4,HIGH);
      //Serial.println(ps2x.Analog(PSS_LY));
      //Serial.println("ESQUERDA");
    }
  }
}

/* ========== ROTINAS DE INTERRUPÇÃO ========== */

// não apagar esta rotina, é necessária para o funcionamento
ISR(TIMER2_OVF_vect)
{
  TCNT2=100;          // Reinicializa o registrador do Timer2
}
