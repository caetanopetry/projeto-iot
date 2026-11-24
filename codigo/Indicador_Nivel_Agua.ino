#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <EEPROM.h>

// ---------- Configuração do LCD I2C Físico ----------
#define ende 0x27 
#define col 16
#define lin 2

LiquidCrystal_I2C lcd(ende, col, lin);

// ---------- Pinos ----------
const int SENSOR_PIN = A0;

// LEDs em ordem acumulativa: Verde 1, Verde 2, Amarelo, Vermelho
const int LED_PINS[4] = {9, 10, 11, 12};
const int BUZZER_PIN = 13;

// Calibração do Sensor Real (Teto máximo de leitura quando submerso)
const int VALOR_MAX_SENSOR = 600; 

// ---------- Teclado Matricial 4x3 ----------
const byte LINHAS = 4; // 4 linhas
const byte COLUNAS = 3; // 3 colunas

// Mapa de teclas do teclado 4x3 real
char mapaTeclas[LINHAS][COLUNAS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

// Pinos das Linhas (Cabos azuis): 8, 7, 6, 5
byte pinosLinhas[LINHAS]   = {8, 7, 6, 5};
// Pinos das Colunas (Cabos roxos): 4, 3, 2
byte pinosColunas[COLUNAS] = {4, 3, 2};

Keypad teclado = Keypad(makeKeymap(mapaTeclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS);

// ---------- Parâmetros e estados ----------
int limiteCheio = 480; // Equivalente a 80% do teto de 600
bool buzzerMudo = false;

unsigned long ultimoBeep = 0;
bool estadoBeep = false;
unsigned long ultimaTela = 0;

void mostraAviso(const char* linha1, const char* linha2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(linha1);
  lcd.setCursor(0, 1);
  lcd.print(linha2);
  delay(1000);
  lcd.clear();
}

// Acende os LEDs proporcionalmente (25%, 50%, 75% e Limite Cheio)
void atualizaLEDs(int leitura) {
  int limites[4] = {150, 300, 450, limiteCheio};
  for (int i = 0; i < 4; i++) {
    digitalWrite(LED_PINS[i], leitura >= limites[i] ? HIGH : LOW);
  }
}

// Bips do buzzer ativam a partir de 75% (450) e aceleram até 100% (600)
void atualizaBuzzer(int leitura) {
  if (buzzerMudo || leitura < 450) {
    noTone(BUZZER_PIN);
    estadoBeep = false;
    return;
  }

  unsigned long agora = millis();

  if (leitura < limiteCheio) {
    // Alerta moderado (entre 75% e o limite configurado)
    if (estadoBeep && (agora - ultimoBeep >= 60)) {
      estadoBeep = false;
      noTone(BUZZER_PIN);
      ultimoBeep = agora;
    } else if (!estadoBeep && (agora - ultimoBeep >= 500)) {
      estadoBeep = true;
      tone(BUZZER_PIN, 1000);
      ultimoBeep = agora;
    }
  } else {
    // Alerta crítico (acima do limite configurado até o teto de 600)
    int intervaloPausa = map(constrain(leitura, limiteCheio, VALOR_MAX_SENSOR), limiteCheio, VALOR_MAX_SENSOR, 160, 35);

    if (estadoBeep && (agora - ultimoBeep >= 50)) {
      estadoBeep = false;
      noTone(BUZZER_PIN);
      ultimoBeep = agora;
    } else if (!estadoBeep && (agora - ultimoBeep >= (unsigned long)intervaloPausa)) {
      estadoBeep = true;
      tone(BUZZER_PIN, 2100);
      ultimoBeep = agora;
    }
  }
}

void trataTecla(char tecla, int leitura) {
  switch (tecla) {
    case '1':
      buzzerMudo = true;
      noTone(BUZZER_PIN);
      estadoBeep = false;
      mostraAviso("Buzzer: MUDO", "Tecla 2 reativa");
      break;

    case '2':
      buzzerMudo = false;
      mostraAviso("Buzzer: ATIVO", "Alertas ligados");
      break;

    case '3':
      limiteCheio = leitura;
      EEPROM.put(0, limiteCheio);
      mostraAviso("Limite salvo!", "Novo max gravado");
      break;

    // Como o teclado 4x3 não tem a tecla D, usamos a tecla '#' para a ajuda
    case '#':
      mostraAviso("1:Mudo 2:Som", "3:Grava Limite");
      break;
  }
}

void setup() {
  lcd.init();          
  lcd.backlight();

  for (int i = 0; i < 4; i++) {
    pinMode(LED_PINS[i], OUTPUT);
  }
  pinMode(BUZZER_PIN, OUTPUT);

  int salvo;
  EEPROM.get(0, salvo);
  if (salvo > 100 && salvo <= VALOR_MAX_SENSOR) {
    limiteCheio = salvo;
  }

  // Aviso adaptado para a tecla '#' no lugar do 'D'
  mostraAviso("Sistema Pronto", "# = Ajuda teclas");
}

void loop() {
  int leitura = analogRead(SENSOR_PIN);

  // Mapeia a porcentagem baseada no teto real de 600
  int porcentagem = map(leitura, 0, VALOR_MAX_SENSOR, 0, 100);
  porcentagem = constrain(porcentagem, 0, 100); // Trava estritamente de 0 a 100%

  char tecla = teclado.getKey();
  if (tecla) {
    trataTecla(tecla, leitura);
  }

  atualizaLEDs(leitura);
  atualizaBuzzer(leitura);

  if (millis() - ultimaTela >= 120) {
    ultimaTela = millis();

    // Linha 0: Leitura bruta e status do som
    lcd.setCursor(0, 0);
    lcd.print("Leitura:");
    lcd.print(leitura);
    lcd.print("   ");
    lcd.setCursor(11, 0);
    lcd.print(buzzerMudo ? "  MUDO" : " SOM");

    // Linha 1: Porcentagem ou Alerta de Perigo
    lcd.setCursor(0, 1);
    if (leitura >= limiteCheio) {
      lcd.print("PERIGO: 100%    ");
    } else {
      lcd.print("Nivel: ");
      lcd.print(porcentagem);
      lcd.print("%        ");
    }
  }
}



