#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pino do botão nativo (KEY) na Black Pill F411
const int BUTTON_PIN = PA0; 

// Variáveis para controle do estado do botão
bool estadoBotaoAtual{false};
bool estadoBotaoAnterior{true}; // Começa em HIGH devido ao pull-up elétrico da placa

// Variáveis para o contador e debounce por software
int contadorPressionamentos{};
unsigned long tempoUltimoDebounce{};
const unsigned long DELAY_DEBOUNCE{5}; // 5ms é suficiente para filtragem mecânica

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
  lcd.print("Varredura Botao");
  lcd.setCursor(0, 1);
  lcd.print("Cliques: 0");

  // Configura o pino do botão como INPUT
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void leBotao(bool leitura) {
// 2. Se o estado mudou (por ruído ou clique), reseta o temporizador de debounce
  if (leitura != estadoBotaoAnterior) {
    tempoUltimoDebounce = millis();
  }

  // 3. Se passou tempo suficiente, a leitura física estabilizou
  if ((millis() - tempoUltimoDebounce) > DELAY_DEBOUNCE) {
    
    // Se o estado estabilizado for diferente do estado que tínhamos guardado
    if (leitura != estadoBotaoAtual) {
      estadoBotaoAtual = leitura;

      // Detecta a borda de descida: o botão mudou para LOW (foi pressionado)
      if (estadoBotaoAtual == LOW) { 
        // Atualiza o display LCD imediatamente
        lcd.setCursor(9, 1);
        lcd.print("       "); // Limpa o número anterior
        lcd.setCursor(9, 1);
        lcd.print(++contadorPressionamentos);
      }
    }
  }
  // 4. Salva a leitura para o próximo ciclo do loop
  estadoBotaoAnterior = leitura;
}

void loop() {
  // 1. Lê o estado atual do botão
  leBotao(digitalRead(BUTTON_PIN));
}