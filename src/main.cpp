#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SOLENOIDE  PC13
#define SENSOR_1  PB0
#define SENSOR_2  PB1

#define flagTrava  PA1

#define GIRO_HORARIO  PA7
#define GIRO_ANTI_HORARIO  PA6
#define GIRO_DESEMPATE  PA5

const char msgPadraoL1[17]{"    LION TEC    "};
const char msgPadraoL2[17]{"CONTROLE ACESSO"};

class Sensores {
private:
  uint8_t m_solenoide{};
  uint8_t m_sensor1{};
  uint8_t m_sensor2{};

public:
  Sensores(uint8_t solenoide, uint8_t sensor1, uint8_t sensor2)
  : m_solenoide{solenoide}
  , m_sensor1{sensor1}
  , m_sensor2{sensor2}
  {}

  void inicializa() {
    pinMode(m_solenoide, OUTPUT_OPENDRAIN);
    pinMode(m_sensor1, INPUT_PULLUP);
    pinMode(m_sensor2, INPUT_PULLUP);
    digitalWrite(m_solenoide, HIGH);
  }

  void setSolenoide(PinStatus s) {return digitalWrite(m_solenoide, s);}
  uint8_t getSensor1() {return static_cast<uint8_t>(digitalRead(m_sensor1));}
  uint8_t getSensor2() {return static_cast<uint8_t>(digitalRead(m_sensor2));}
};

class ConfigGiro {
private:
  uint8_t m_horario{};
  uint8_t m_antiHorario{};
  uint8_t m_desempate{};
  uint8_t m_flagTrava{};

public:
  ConfigGiro(uint8_t horario, uint8_t antiHorario, uint8_t desempate, uint8_t flag) 
  : m_horario{static_cast<uint8_t>(digitalRead(horario))}
  , m_antiHorario{static_cast<uint8_t>(digitalRead(antiHorario))}
  , m_desempate{static_cast<uint8_t>(digitalRead(desempate))} 
  , m_flagTrava{static_cast<uint8_t>(digitalRead(flag))}
  {}

  void inicializa() {
    pinMode(m_horario, INPUT);
    pinMode(m_antiHorario, INPUT);
    pinMode(m_desempate, INPUT);
  }

  uint8_t getHorario() {return m_horario;}
  uint8_t getAntiHorario() {return m_antiHorario;}
  uint8_t getGiroDesempate() {return m_desempate;}
  uint8_t getFlagTrava() {return m_flagTrava;}

  //bool operator==(const PinStatus& status) {return *this == status;}
};

// Inicializa o display LCD no endereço 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

Sensores sensorStatus{SOLENOIDE, SENSOR_1, SENSOR_2};
ConfigGiro conf{GIRO_HORARIO, GIRO_ANTI_HORARIO, GIRO_DESEMPATE, flagTrava};

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

  pinMode(flagTrava, INPUT);

  sensorStatus.inicializa();
  conf.inicializa();
  
  escreveDisplay(msgPadraoL1, msgPadraoL2);
}

// Aciona o solenoide enquanto sensor estiver cortado
void loopSensores() {
  while (sensorStatus.getSensor1() == LOW || sensorStatus.getSensor2() == LOW);
  sensorStatus.setSolenoide(HIGH); 
}

void escreveDisplay(const char* linha1, const char* linha2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(linha1);
  lcd.setCursor(0, 1);
  lcd.print(linha2);
}

uint8_t giroHorario{conf.getHorario()};
uint8_t giroAntiHorario{conf.getAntiHorario()};
uint8_t giroDesempate{conf.getGiroDesempate()};


void leSensor(uint8_t leitura1, uint8_t leitura2) {
  uint8_t trava{conf.getFlagTrava()};
  ////////////////// SENDOR 1 ////////////////////////
  if (!leitura1) {
    if(giroHorario && giroAntiHorario) {
      if(trava) {
        sensorStatus.setSolenoide(LOW);
        escreveDisplay(" ACESSO NEGADO", "");
      }  
      else if (giroDesempate) {
        escreveDisplay("    Entrando", "");
      }
      else {
        escreveDisplay("     Saindo", "");
      }
    }
    else if(!giroHorario && !giroAntiHorario) {
      if(giroDesempate) {
        escreveDisplay("SEMPRE LIBERADO", "    Entrando");
      }
      else {
        escreveDisplay("SEMPRE LIBERADO", "     Saindo"); 
      }   
    }
    else if (giroHorario) {
      if (trava) {
        sensorStatus.setSolenoide(LOW);
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
  if (!leitura2) {
    if(giroAntiHorario && giroHorario) {
      if(trava == HIGH) {
        sensorStatus.setSolenoide(LOW);
        escreveDisplay(" ACESSO NEGADO", "");
      }  
      else if (!giroDesempate) {
        escreveDisplay("    Entrando", "");
      }
      else {
        escreveDisplay("     Saindo", "");
      }
    }
    else if(!giroAntiHorario  && !giroHorario) {
      if(!giroDesempate) {
        escreveDisplay("SEMPRE LIBERADO", "    Entrando");
      }
      else {
        escreveDisplay("SEMPRE LIBERADO", "     Saindo"); 
      }   
    }
    else if (giroAntiHorario) {
      if (trava) {
        sensorStatus.setSolenoide(LOW);
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
  leSensor(sensorStatus.getSensor1(), sensorStatus.getSensor2()); 
}