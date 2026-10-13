#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SOLENOIDE  PC13
#define SENSOR_1  PB0
#define SENSOR_2  PB1

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
  bool m_trava{};

public:
  Sensores() = default;
  Sensores(uint8_t solenoide, uint8_t sensor1, uint8_t sensor2, bool trava = true)
  : m_solenoide{solenoide}
  , m_sensor1{sensor1}
  , m_sensor2{sensor2}
  , m_trava{trava}
  {}

  void inicializar() {
    pinMode(m_solenoide, OUTPUT_OPENDRAIN);
    pinMode(m_sensor1, INPUT_PULLUP);
    pinMode(m_sensor2, INPUT_PULLUP);
    digitalWrite(m_solenoide, HIGH);
  }

  void setSolenoide(PinStatus s) {return digitalWrite(m_solenoide, s);}
  uint8_t getSensor1() {return digitalRead(m_sensor1);}
  uint8_t getSensor2() {return digitalRead(m_sensor2);}
  void setFlagTrava(bool flag) {m_trava = flag;}
  bool getFlagTrava() {return m_trava;}
};

// Declarações encaminhamento
void loopSensores();
void escreveDisplay(const char* linha1, const char* linha2);

class SerialPc : public Sensores {
private:
    static constexpr uint8_t TAMANHO_BUFFER = 64; 
    char m_bufferMensagem[TAMANHO_BUFFER];
    uint8_t m_indiceBuffer;
    
    bool m_descarta;
    unsigned long m_tempoUltimoByte; // Registra o tempo do último byte que passou pelo cabo

public:
    SerialPc() 
        : m_indiceBuffer{0}, m_descarta{false}, m_tempoUltimoByte{0} {}

    void inicializar() {
        Serial1.begin(9600);
    }

    void separaDadosEdecide(char delimitador) {
        char linha1[17]; 
        char linha2[48]; 

        for (int i = 0; i < 16; i++) {
            linha1[i] = m_bufferMensagem[5 + i]; // 5 = cabeçalho "0Lion"
        }
        linha1[16] = '\0'; 

        int idxlinha2 = 0;
        int idxOrigem = 21; // 16 caracteres da linha1 + 5 do cabeçalho
        // Enquanto não encontrar o delimitador ou o final da string, copia os caracteres para linha2
        while (m_bufferMensagem[idxOrigem] != 'I' && m_bufferMensagem[idxOrigem] != '>' && m_bufferMensagem[idxOrigem] != '\0') {
            linha2[idxlinha2] = m_bufferMensagem[idxOrigem];
            idxlinha2++;
            idxOrigem++;
        }
        linha2[idxlinha2] = '\0'; 

        Serial.println(linha1);
        Serial.println(linha2);
        escreveDisplay(linha1, linha2);
        
        unsigned long tempoMsgPresa{millis()};
        if (delimitador == 'I') {
            setFlagTrava(false);
            while (digitalRead(SENSOR_1) && digitalRead(SENSOR_2) && millis() - tempoMsgPresa < 10000);
            loopSensores();
            escreveDisplay(msgPadraoL1, msgPadraoL2);
            setFlagTrava(true);
        }
        else {
            while (digitalRead(SENSOR_1) && digitalRead(SENSOR_2) && millis() - tempoMsgPresa < 5000);
            if(millis() - tempoMsgPresa < 10000)
                digitalWrite(SOLENOIDE, LOW);
            loopSensores();
            escreveDisplay(msgPadraoL1, msgPadraoL2);
        }
    }

    void entradaDados() {
        // Desativa a trava de descarte se já se passaram 15ms desde o último byte recebido
        if (m_descarta && (millis() - m_tempoUltimoByte > 15)) {
            m_descarta = false; // Desativa a trava e aceita novos eventos futuros
        }
        // Lê entrada serial 
        while (Serial1.available() > 0) {
            char caractereAtual = Serial1.read();
            m_tempoUltimoByte = millis(); 
            // Se a trava estiver ativa, apenas consome o byte e joga no lixo
            if (m_descarta) {
                continue; 
            }

            if (m_indiceBuffer >= TAMANHO_BUFFER - 1) {
                m_indiceBuffer = 0;
            }

            m_bufferMensagem[m_indiceBuffer] = caractereAtual;
            m_indiceBuffer++;
            m_bufferMensagem[m_indiceBuffer] = '\0';

            // Terminador trava ou libera
            if (caractereAtual == 'I' || caractereAtual == '>') {
                separaDadosEdecide(caractereAtual); 
                m_indiceBuffer = 0; 
                
                // ATIVAÇÃO IMEDIATA: Entra em modo descarte direto, sem testar o .available()
                // Tudo o que chegar na sequência será ignorado até o silêncio de 15ms acontecer
                m_descarta = true; 
            }
        }
    }
};

class ConfigGiro {
private:
  uint8_t m_horario{};
  uint8_t m_antiHorario{};
  uint8_t m_desempate{};

public:
  ConfigGiro(uint8_t horario, uint8_t antiHorario, uint8_t desempate) 
  : m_horario{static_cast<uint8_t>(digitalRead(horario))}
  , m_antiHorario{static_cast<uint8_t>(digitalRead(antiHorario))}
  , m_desempate{static_cast<uint8_t>(digitalRead(desempate))} 
  {}

  void inicializar() {
    pinMode(m_horario, INPUT);
    pinMode(m_antiHorario, INPUT);
    pinMode(m_desempate, INPUT);
  }

  uint8_t getHorario() {return m_horario;}
  uint8_t getAntiHorario() {return m_antiHorario;}
  uint8_t getGiroDesempate() {return m_desempate;}
};

LiquidCrystal_I2C lcd(0x27, 16, 2); // Inicializa o display LCD no endereço 0x27
SerialPc serialPC{};
Sensores sensorStatus{SOLENOIDE, SENSOR_1, SENSOR_2};
ConfigGiro conf{GIRO_HORARIO, GIRO_ANTI_HORARIO, GIRO_DESEMPATE};

void setup() {
  Serial.begin(115200); // monitor serial
  // Inicializa o I2C nos pinos PB7 (SDA) e PB6 (SCL)
  Wire.begin(PB7, PB6);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("    Lion Tec    ");
  lcd.setCursor(0, 1);
  lcd.print("    Rev. 3.0    ");
  delay(5000);

  serialPC.inicializar();
  sensorStatus.inicializar();
  conf.inicializar();
  
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

void leSensor(uint8_t leitura1, uint8_t leitura2) {
  // Grava a configuração do giro
  static uint8_t giroHorario{conf.getHorario()};
  static uint8_t giroAntiHorario{conf.getAntiHorario()};
  static uint8_t giroDesempate{conf.getGiroDesempate()};
  static uint8_t trava{sensorStatus.getFlagTrava()};

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
  // Lê o buffer serial
  serialPC.entradaDados();
  // Lê o estado atual dos sensores
  leSensor(sensorStatus.getSensor1(), sensorStatus.getSensor2()); 
}