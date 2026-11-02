// =====================================================================
// ARMÁRIO INTELIGENTE - Teclado com INTERRUPÇÃO + senha + LEDs
//                       + BUZZER + TRANCA (relé)
// =====================================================================

// ---------- PINOS ----------
const byte LINHA_PINO[4]  = {4, 5, 6, 7};    // linhas do teclado (entradas, D4-D7)
const byte COLUNA_PINO[4] = {8, 9, 10, 11};  // colunas do teclado (saídas, D8-D11)
const int PINO_RELE = 12;                    // sinal S do relé no D12
const int BUZZER = 13;                       // buzzer: + no D13, - no GND
const int LED_VERDE = A0;
const int LED_VERMELHO = A1;

// Mapa das teclas: [linha][coluna]
const char TECLAS[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// ---------- SENHA E TEMPOS ----------
const String SENHA_CORRETA = "1234";      // senha provisória
String digitado = "";
const unsigned long TEMPO_VERDE = 3000;   // LED verde aceso por 3 s
const unsigned long TEMPO_VERMELHO = 1500;// LED vermelho aceso por 1,5 s
const unsigned long TEMPO_TRANCA = 1000;  // relé ligado por 1 s (pulso)
bool ledVerdeAceso = false;
bool ledVermelhoAceso = false;
unsigned long inicioVerde = 0;
unsigned long inicioVermelho = 0;

// ---------- INTERRUPÇÃO ----------
// "volatile" avisa o compilador que esta variável muda dentro da interrupção.
volatile bool teclaPossivel = false;

// Roda automaticamente quando D4-D7 mudam de estado. Só marca a bandeira.
ISR(PCINT2_vect) {
  teclaPossivel = true;
}

// ---------- SETUP ----------
void setup() {
  // Relé primeiro: garante a fechadura travada desde o início
  pinMode(PINO_RELE, OUTPUT);
  digitalWrite(PINO_RELE, LOW);

  Serial.begin(9600);

  // Linhas: entradas com pull-up (HIGH quando nada é apertado)
  for (int r = 0; r < 4; r++) {
    pinMode(LINHA_PINO[r], INPUT_PULLUP);
  }

  // Colunas: saídas em LOW (permite a interrupção detectar o aperto)
  for (int c = 0; c < 4; c++) {
    pinMode(COLUNA_PINO[c], OUTPUT);
    digitalWrite(COLUNA_PINO[c], LOW);
  }

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, LOW);
  pinMode(BUZZER, OUTPUT);

  // Liga a interrupção de mudança de pino só para D4-D7
  noInterrupts();
  PCICR  |= (1 << PCIE2);      // habilita o grupo PCINT2 (pinos D0-D7)
  PCMSK2 |= 0b11110000;        // dentro do grupo, só D4, D5, D6 e D7
  interrupts();

  Serial.println("Digite a senha e aperte # para confirmar. * apaga.");
}

// ---------- LOOP ----------
void loop() {
  atualizaLeds();              // apaga os LEDs na hora certa, sem travar

  if (!teclaPossivel) {        // nenhuma interrupção: nada a fazer
    return;
  }
  teclaPossivel = false;

  delay(20);                   // antirrebote: espera o contato estabilizar
  char tecla = lerTecla();     // descobre qual tecla foi apertada
  if (tecla == 0) {            // ruído ou soltura da tecla: ignora
    return;
  }

  somClique();                 // bip curto: "recebi a tecla"
  processarTecla(tecla);

  esperarSoltar();             // espera o dedo sair da tecla
  teclaPossivel = false;       // descarta sinais gerados ao soltar
}

// ---------- LEITURA DA TECLA (varredura) ----------
// Testa uma coluna por vez: coloca a coluna em LOW e vê qual linha cai.
char lerTecla() {
  for (int c = 0; c < 4; c++) {
    for (int k = 0; k < 4; k++) {
      pinMode(COLUNA_PINO[k], INPUT_PULLUP);   // todas as colunas "soltas"
    }
    pinMode(COLUNA_PINO[c], OUTPUT);           // só a coluna c vai para LOW
    digitalWrite(COLUNA_PINO[c], LOW);
    delayMicroseconds(50);

    for (int r = 0; r < 4; r++) {
      if (digitalRead(LINHA_PINO[r]) == LOW) { // linha caiu: tecla [r][c]
        char tecla = TECLAS[r][c];
        restauraColunas();
        return tecla;
      }
    }
  }
  restauraColunas();
  return 0;                                    // nenhuma tecla encontrada
}

// Volta todas as colunas para LOW (estado de espera da interrupção)
void restauraColunas() {
  for (int c = 0; c < 4; c++) {
    pinMode(COLUNA_PINO[c], OUTPUT);
    digitalWrite(COLUNA_PINO[c], LOW);
  }
}

// Espera até nenhuma linha estar em LOW (tecla solta)
void esperarSoltar() {
  while (true) {
    bool algumaApertada = false;
    for (int r = 0; r < 4; r++) {
      if (digitalRead(LINHA_PINO[r]) == LOW) {
        algumaApertada = true;
      }
    }
    if (!algumaApertada) {
      break;
    }
    atualizaLeds();
    delay(10);
  }
}

// ---------- LÓGICA DA SENHA ----------
void processarTecla(char tecla) {
  if (tecla == '#') {
    if (digitado == SENHA_CORRETA) {
      acessoLiberado();
    } else {
      acessoNegado();
    }
    digitado = "";
  }
  else if (tecla == '*') {
    digitado = "";
    Serial.println("Digitação apagada.");
  }
  else {
    digitado += tecla;
    Serial.print("Digitando... (");
    Serial.print(digitado.length());
    Serial.println(" teclas)");
  }
}

void acessoLiberado() {
  Serial.println("ACESSO LIBERADO");
  digitalWrite(LED_VERDE, HIGH);
  ledVerdeAceso = true;
  inicioVerde = millis();

  somLiberado();               // toca o som de sucesso
  destravarTranca();           // manda o pulso para o relé
}

void acessoNegado() {
  Serial.println("ACESSO NEGADO");
  digitalWrite(LED_VERMELHO, HIGH);
  ledVermelhoAceso = true;
  inicioVermelho = millis();
  somNegado();                 // relé continua desligado: tranca travada
}

// ---------- TRANCA ----------
// Liga o relé por 1 s (12 V chegam à fechadura) e desliga de novo
void destravarTranca() {
  Serial.println("Tranca: relé LIGADO");
  digitalWrite(PINO_RELE, HIGH);
  delay(TEMPO_TRANCA);
  digitalWrite(PINO_RELE, LOW);
  Serial.println("Tranca: relé DESLIGADO");
}

// Apaga os LEDs quando o tempo passa (sem usar delay)
void atualizaLeds() {
  if (ledVerdeAceso && millis() - inicioVerde >= TEMPO_VERDE) {
    digitalWrite(LED_VERDE, LOW);
    ledVerdeAceso = false;
  }
  if (ledVermelhoAceso && millis() - inicioVermelho >= TEMPO_VERMELHO) {
    digitalWrite(LED_VERMELHO, LOW);
    ledVermelhoAceso = false;
  }
}

// ---------- SONS ----------
// tone(pino, frequência em Hz, duração em ms)
void somClique() {
  tone(BUZZER, 2000, 60);      // bip agudo e curto
}

void somLiberado() {
  tone(BUZZER, 1200, 150);     // dois bips subindo = sucesso
  delay(200);
  tone(BUZZER, 1800, 300);
  delay(350);
}

void somNegado() {
  tone(BUZZER, 300, 700);      // som grave e longo = erro
  delay(750);
}