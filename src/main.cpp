#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

constexpr uint8_t PINO_SOLENOIDE = PC13; // Pino para o solenoide
constexpr uint8_t SENSOR_HORARIO = PA0; 
constexpr uint8_t SENSOR_ANTIHORARIO = PA1;

// Variáveis para controle do estado do botão
bool estadoSensorHorario{false};
bool estadoAnteriorSensorHorario{true}; // Começa em HIGH devido ao pull-up elétrico da placa
bool estadoSensorAntihorario{false};
bool estadoAnteriorSensorAntihorario{true}; // Começa em HIGH devido ao pull-up elétrico da placa
bool flagTrava{true}; // Variável para decidir a trava solenoide

// Variáveis para o contador e debounce por software
int contadorPressionamentos{};
unsigned long tempoUltimoDebounce{};
constexpr unsigned long DELAY_DEBOUNCE{5}; // 5ms é suficiente para filtragem mecânica

// Inicializa o display LCD no endereço 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

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
  pinMode(SENSOR_HORARIO, INPUT_PULLUP);
  pinMode(SENSOR_ANTIHORARIO, INPUT_PULLUP);

  mensagemPadrao();
}

void solenoide(PinStatus estado) {
  // Aciona o solenoide enquanto sensor estiver cortado
  digitalWrite(PINO_SOLENOIDE, estado);
  while (SENSOR_HORARIO == LOW || SENSOR_ANTIHORARIO == LOW);
  digitalWrite(PINO_SOLENOIDE, LOW);
}

void escreveDisplay(const char* linha1, const char* linha2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(linha1);
  lcd.setCursor(0, 1);
  lcd.print(linha2);
}

void mensagemPadrao() {
  escreveDisplay("      Olá!      ", "   Bem-vindo");
}

void leSensor(bool leitura) {
// 2. Se o estado mudou (por ruído ou clique), reseta o temporizador de debounce
  if (leitura != estadoAnteriorSensorHorario) {
    tempoUltimoDebounce = millis();
  }

  // 3. Se passou tempo suficiente, a leitura física estabilizou
  if ((millis() - tempoUltimoDebounce) > DELAY_DEBOUNCE) {
    
    // Se o estado estabilizado for diferente do estado que tínhamos guardado
    if (leitura != estadoSensorHorario) {
      estadoSensorHorario = leitura;

      // Detecta a borda de descida: o sensor mudou para LOW (foi cortado)
      if (estadoSensorHorario == false && SENSOR_HORARIO == HIGH) { 
        if(flagTrava == true) {
          escreveDisplay(" ACESSO  NEGADO ", "                ");
          solenoide(HIGH);
          mensagemPadrao();
			  }
		    else if(flagTrava == false) {
				  escreveDisplay("    Entrando    ", "                ");
          solenoide(LOW);
          mensagemPadrao();
			  }        
      }
      if (estadoSensorHorario == false && SENSOR_HORARIO == LOW) { 
				  escreveDisplay("     Saindo     ", "");
          mensagemPadrao();
			}        
    }
  }
  // 4. Salva a leitura para o próximo ciclo do loop
  estadoAnteriorSensorHorario = leitura;
}

void loop() {
  // 1. Lê o estado atual do botão
  leSensor(digitalRead(SENSOR_HORARIO));
}