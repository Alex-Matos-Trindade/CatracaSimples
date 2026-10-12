#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const PinStatus& Controlar{HIGH};
const PinStatus& Liberar{LOW};

const char* msgPadraoL1{"    LION TEC    "};
const char* msgPadraoL2{"CONTROLE ACESSO"};

namespace Pinos
{
  constexpr uint8_t PINO_SOLENOIDE = PC13; 
  constexpr uint8_t SENSOR_1 = PB0; 
  constexpr uint8_t SENSOR_2 = PB1;

  constexpr uint8_t flagTrava = PA1;
  constexpr uint8_t GIRO_HORARIO = PA7;  
  constexpr uint8_t GIRO_ANTI_HORARIO = PA6;
  constexpr uint8_t GIRO_DESEMPATE = PA5; // true = trava horario, false = trava anti horario
} 

struct Giros {
  PinStatus giroHorario{};
  PinStatus giroAntiHorario{};
  PinStatus giroDesempate{};

  bool operator==(const PinStatus& status) {return *this == status;} 
};

// Inicializa o display LCD no endereço 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

void escreveDisplay(const char* linha1, const char* linha2); // Declaração encaminhamento

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

  pinMode(Pinos::PINO_SOLENOIDE, OUTPUT_OPENDRAIN);
  pinMode(Pinos::SENSOR_1, INPUT_PULLUP);
  pinMode(Pinos::SENSOR_2, INPUT_PULLUP);
  digitalWrite(Pinos::PINO_SOLENOIDE, HIGH); // Garante que o solenoide comece desligado

  pinMode(Pinos::flagTrava, INPUT);
  pinMode(Pinos::GIRO_HORARIO, INPUT);
  pinMode(Pinos::GIRO_ANTI_HORARIO, INPUT);
  pinMode(Pinos::GIRO_DESEMPATE, INPUT);

  escreveDisplay(msgPadraoL1, msgPadraoL2);
}

// Aciona o solenoide enquanto sensor estiver cortado
void loopSensores() {
  while (digitalRead(Pinos::SENSOR_1) == LOW || digitalRead(Pinos::SENSOR_2) == LOW);
  digitalWrite(Pinos::PINO_SOLENOIDE, HIGH); 
}

void escreveDisplay(const char* linha1, const char* linha2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(linha1);
  lcd.setCursor(0, 1);
  lcd.print(linha2);
}

Giros giro{digitalRead(Pinos::GIRO_HORARIO), digitalRead(Pinos::GIRO_ANTI_HORARIO), digitalRead(Pinos::GIRO_DESEMPATE)};

void leSensor(PinStatus leitura1, PinStatus leitura2) {
  PinStatus trava{digitalRead(Pinos::flagTrava)};
  ////////////////// SENDOR 1 ////////////////////////
  if (leitura1 == LOW) {
    if(giro.giroHorario == Controlar && giro.giroAntiHorario == Controlar) {
      if(trava == HIGH) {
        digitalWrite(Pinos::PINO_SOLENOIDE, LOW);
        escreveDisplay(" ACESSO NEGADO", "");
      }  
      else if (giro.giroDesempate == Controlar) {
        escreveDisplay("    Entrando", "");
      }
      else {
        escreveDisplay("     Saindo", "");
      }
    }
    else if(giro.giroHorario == Liberar && giro.giroAntiHorario == Liberar) {
      if(giro.giroDesempate == Controlar) {
        escreveDisplay("SEMPRE LIBERADO", "    Entrando");
      }
      else {
        escreveDisplay("SEMPRE LIBERADO", "     Saindo"); 
      }   
    }
    else if (giro.giroHorario == Controlar) {
      if (trava == HIGH) {
        digitalWrite(Pinos::PINO_SOLENOIDE, LOW);
        escreveDisplay(" ACESSO NEGADO", "");
      }
      else {
        escreveDisplay("    Entrando", "");
      }         
    }
    else {
      escreveDisplay("     Saindo", "");
    }

    loopSensores();
    escreveDisplay(msgPadraoL1, msgPadraoL2);
  }
  ////////////////// SENDOR 2 ////////////////////////
  if (leitura2 == LOW) {
    if(giro.giroAntiHorario == Controlar && giro.giroHorario == Controlar) {
      if(trava == HIGH) {
        digitalWrite(Pinos::PINO_SOLENOIDE, LOW);
        escreveDisplay(" ACESSO NEGADO", "");
      }  
      else if (giro.giroDesempate == Liberar) {
        escreveDisplay("    Entrando", "");
      }
      else {
        escreveDisplay("     Saindo", "");
      }
    }
    else if(giro.giroAntiHorario == Liberar && giro.giroHorario == Liberar) {
      if(giro.giroDesempate == Liberar) {
        escreveDisplay("SEMPRE LIVERADO", "    Entrando");
      }
      else {
        escreveDisplay("SEMPRE LIVERADO", "     Saindo"); 
      }   
    }
    else if (giro.giroAntiHorario == Controlar) {
      if (trava == HIGH) {
        digitalWrite(Pinos::PINO_SOLENOIDE, LOW);
        escreveDisplay(" ACESSO NEGADO", "");
      }
      else {
        escreveDisplay("    Entrando", "");
      }         
    }
    else {
      escreveDisplay("     Saindo", "");
    } 
    
    loopSensores();
    escreveDisplay(msgPadraoL1, msgPadraoL2);
  }     
}

void loop() {
  // 1. Lê o estado atual dos sensores
  leSensor(digitalRead(Pinos::SENSOR_1), digitalRead(Pinos::SENSOR_2)); 
}