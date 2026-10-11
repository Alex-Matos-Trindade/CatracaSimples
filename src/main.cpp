#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

constexpr uint8_t PINO_SOLENOIDE = PC13; // Pino para o solenoide
constexpr uint8_t SENSOR_1 = PB0; 
constexpr uint8_t SENSOR_2 = PB1;

// Variáveis para controle do estado do sensor
bool estadoSensor1{false};
bool estadoAnteriorSensor1{true}; 
bool estadoSensor2{false};
bool estadoAnteriorSensor2{true}; 
//bool flagTrava{true}; // Variável para decidir a trava solenoide

constexpr uint8_t flagTrava = PA1;
constexpr uint8_t GIRO_HORARIO = PA7;  
constexpr uint8_t GIRO_ANTI_HORARIO = PA6;
constexpr uint8_t GIRO_DESEMPATE = PA5; // true = trava horario, false = trava anti horario

// Variáveis para o contador e debounce por software
int contadorPressionamentos{};
unsigned long tempoUltimoDebounce{};
constexpr unsigned long DELAY_DEBOUNCE{}; // 5ms é suficiente para filtragem mecânica

const PinStatus& Controlar{HIGH};
const PinStatus& Liberar{LOW};

// Inicializa o display LCD no endereço 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

void mensagemPadrao();
void setup() {
  // Inicializa o I2C nos pinos PB7 (SDA) e PB6 (SCL)
  Wire.begin(PB7, PB6);
  
  // Inicializa e liga o LCD
  lcd.init();
  lcd.backlight();
  
  // Tela inicial
  lcd.setCursor(0, 0);
  lcd.print("    Lion Tec    ");
  lcd.setCursor(0, 1);
  lcd.print("    Rev. 3.0    ");
  delay(5000);

  pinMode(PINO_SOLENOIDE, OUTPUT_OPENDRAIN);
  pinMode(SENSOR_1, INPUT_PULLUP);
  pinMode(SENSOR_2, INPUT_PULLUP);
  digitalWrite(PINO_SOLENOIDE, HIGH); // Garante que o solenoide comece desligado

  pinMode(flagTrava, INPUT);
  pinMode(GIRO_HORARIO, INPUT);
  pinMode(GIRO_ANTI_HORARIO, INPUT);
  pinMode(GIRO_DESEMPATE, INPUT);

  mensagemPadrao();
}

// Aciona o solenoide enquanto sensor estiver cortado
void loopSensores() {
  while (digitalRead(SENSOR_1) == LOW || digitalRead(SENSOR_2) == LOW) 
  {
    ;
  }
  digitalWrite(PINO_SOLENOIDE, HIGH); 
}


void escreveDisplay(const char* linha1, const char* linha2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(linha1);
  lcd.setCursor(0, 1);
  lcd.print(linha2);
}

void mensagemPadrao() {
  escreveDisplay("    LION TEC    ", "CONTROLE ACESSO");
}

void leSensor(bool leitura1, bool leitura2) {
// 2. Se o estado mudou (por ruído ou clique), reseta o temporizador de debounce
  if (leitura1 != estadoAnteriorSensor1) {
    tempoUltimoDebounce = millis();
  }

  if (leitura2 != estadoAnteriorSensor2) {
    tempoUltimoDebounce = millis();
  }

  // 3. Se passou tempo suficiente, a leitura física estabilizou
  if ((millis() - tempoUltimoDebounce) > DELAY_DEBOUNCE) {
    
    ////////////////// SENDOR 1 ////////////////////////
    // Se o estado estabilizado for diferente do estado que tínhamos guardado
    if (leitura1 != estadoSensor1) {
      estadoSensor1 = leitura1;
      
      if(leitura1 == false && digitalRead(GIRO_HORARIO) == Controlar && digitalRead(GIRO_ANTI_HORARIO)  == Controlar) {
        if(digitalRead(flagTrava) == HIGH) {
          digitalWrite(PINO_SOLENOIDE, LOW);
          escreveDisplay(" ACESSO NEGADO", "");
          loopSensores();
          mensagemPadrao();
        }  
        else if (digitalRead(GIRO_DESEMPATE) == Controlar) {
          escreveDisplay("    Entrando", "");
          loopSensores();
          mensagemPadrao();
        }
        else {
          escreveDisplay("     Saindo", "");
          loopSensores();
          mensagemPadrao();
        }
      }
      else if(leitura1 == false && digitalRead(GIRO_HORARIO) == Liberar && digitalRead(GIRO_ANTI_HORARIO)  == Liberar) {
        if(digitalRead(GIRO_DESEMPATE) == Controlar) {
          escreveDisplay("SEMPRE LIVERADO", "    Entrando");
          loopSensores();
          mensagemPadrao();
        }
        else {
          escreveDisplay("SEMPRE LIVERADO", "     Saindo"); 
          loopSensores();
          mensagemPadrao();
        }   
      }
      else if (leitura1 == false && digitalRead(GIRO_HORARIO) == Controlar) {
        if (digitalRead(flagTrava) == HIGH) {
          digitalWrite(PINO_SOLENOIDE, LOW);
          escreveDisplay(" ACESSO NEGADO", "");
          loopSensores();
          mensagemPadrao();
        }
        else {
          escreveDisplay("    Entrando", "");
          loopSensores();
          mensagemPadrao();
        }         
      }
      else {
        escreveDisplay("     Saindo", "");
        loopSensores();
        mensagemPadrao();
      }
    }
    ////////////////// SENDOR 2 ////////////////////////
    // Se o estado estabilizado for diferente do estado que tínhamos guardado
    if (leitura2 != estadoSensor2) {
      estadoSensor2 = leitura2;
        
      if(leitura2 == false && digitalRead(GIRO_ANTI_HORARIO) == Controlar && digitalRead(GIRO_HORARIO)  == Controlar) {
        if(digitalRead(flagTrava) == HIGH) {
          digitalWrite(PINO_SOLENOIDE, LOW);
          escreveDisplay(" ACESSO NEGADO", "");
          loopSensores();
          mensagemPadrao();
        }  
        else if (digitalRead(GIRO_DESEMPATE) == Liberar) {
          escreveDisplay("    Entrando", "");
          loopSensores();
          mensagemPadrao();
        }
        else {
          escreveDisplay("     Saindo", "");
          loopSensores();
          mensagemPadrao();
        }
      }
      else if(leitura2 == false && digitalRead(GIRO_ANTI_HORARIO) == Liberar && digitalRead(GIRO_HORARIO)  == Liberar) {
        if(digitalRead(GIRO_DESEMPATE) == Liberar) {
          escreveDisplay("SEMPRE LIVERADO", "    Entrando");
          loopSensores();
          mensagemPadrao();
        }
        else {
          escreveDisplay("SEMPRE LIVERADO", "     Saindo"); 
          loopSensores();
          mensagemPadrao();
        }   
      }
      else if (leitura2 == false && digitalRead(GIRO_ANTI_HORARIO) == Controlar) {
        if (digitalRead(flagTrava) == HIGH) {
          digitalWrite(PINO_SOLENOIDE, LOW);
          escreveDisplay(" ACESSO NEGADO", "");
          loopSensores();
          mensagemPadrao();
        }
        else {
          escreveDisplay("    Entrando", "");
          loopSensores();
          mensagemPadrao();
        }         
      }
      else {
        escreveDisplay("     Saindo", "");
        loopSensores();
        mensagemPadrao();
      }    
    }
  }  
  // 4. Salva a leitura para o próximo ciclo do loop
  estadoAnteriorSensor1 = leitura1;
  estadoAnteriorSensor2 = leitura2;  
}

void loop() {
  // 1. Lê o estado atual dos sensores
  leSensor(digitalRead(SENSOR_1), digitalRead(SENSOR_2));
}