#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// ---------- TECLADO ----------
const byte LINHAS = 4;
const byte COLUNAS = 4;
char teclas[LINHAS][COLUNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pinosLinhas[LINHAS]   = {4, 5, 6, 7};
byte pinosColunas[COLUNAS] = {8, 9, 10, 11};
Keypad teclado = Keypad(makeKeymap(teclas), pinosLinhas, pinosColunas,
                        LINHAS, COLUNAS);

// ---------- LEITOR BIOMETRICO ----------
SoftwareSerial serialLeitor(2, 3);          // D2 escuta, D3 fala
Adafruit_Fingerprint leitor = Adafruit_Fingerprint(&serialLeitor);
bool leitorOK = false;

// ---------- LEDs E RELE ----------
const int LED_VERDE = A0;
const int LED_VERMELHO = A1;
const int PINO_RELE = 12;
const int LED_PLACA = LED_BUILTIN;          // LED "L" da placa (pino 13)

// ---------- SENHA E TEMPOS ----------
const String SENHA_CORRETA = "1234";
String digitado = "";
const unsigned long TEMPO_PULSO_RELE = 1000;
const unsigned long TEMPO_VERDE = 3000;
const unsigned long TEMPO_VERMELHO = 1500;

// ---------- BATIMENTO (LED da placa) ----------
unsigned long ultimoPiscar = 0;
bool estadoLedPlaca = false;
unsigned long intervaloPiscar = 500;        // devagar = tudo OK

// Pisca um LED "vezes" vezes (usado no autoteste)
void piscar(int pino, int vezes) {
  for (int i = 0; i < vezes; i++) {
    digitalWrite(pino, HIGH);
    delay(200);
    digitalWrite(pino, LOW);
    delay(200);
  }
}

void setup() {
  digitalWrite(PINO_RELE, LOW);             // rele comeca DESLIGADO
  pinMode(PINO_RELE, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_PLACA, OUTPUT);

  Serial.begin(9600);

  // --- AUTOTESTE: os dois LEDs acendem 1 s ---
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_VERMELHO, HIGH);
  delay(1000);
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, LOW);
  delay(300);

  // --- Procura o leitor (ate 5 tentativas) ---
  leitor.begin(57600);
  for (int tentativa = 1; tentativa <= 5; tentativa++) {
    if (leitor.verifyPassword()) { leitorOK = true; break; }
    delay(500);
  }

  if (leitorOK) {
    Serial.println("Leitor biometrico OK.");
    piscar(LED_VERDE, 2);                   // verde 2x = tudo certo
    intervaloPiscar = 500;                  // batimento devagar
  } else {
    Serial.println("AVISO: leitor NAO encontrado. So a senha vai funcionar.");
    piscar(LED_VERMELHO, 3);                // vermelho 3x = problema no leitor
    intervaloPiscar = 150;                  // batimento rapido
  }
  Serial.println("Pronto: use a senha (#) ou a digital.");
}

// ---------- BATIMENTO ----------
void batimento() {
  // millis() = milissegundos desde que o Arduino ligou.
  // Pisca o LED sem usar delay, sem travar o resto do programa.
  if (millis() - ultimoPiscar >= intervaloPiscar) {
    ultimoPiscar = millis();
    estadoLedPlaca = !estadoLedPlaca;       // inverte: aceso <-> apagado
    digitalWrite(LED_PLACA, estadoLedPlaca);
  }
}

// ---------- ACOES ----------
void acionarRele() {
  digitalWrite(PINO_RELE, HIGH);
  delay(TEMPO_PULSO_RELE);
  digitalWrite(PINO_RELE, LOW);
}

void acessoLiberado() {
  Serial.println("ACESSO LIBERADO");
  digitalWrite(LED_VERDE, HIGH);
  acionarRele();
  delay(TEMPO_VERDE - TEMPO_PULSO_RELE);
  digitalWrite(LED_VERDE, LOW);
}

void acessoNegado() {
  Serial.println("ACESSO NEGADO");
  digitalWrite(LED_VERMELHO, HIGH);
  delay(TEMPO_VERMELHO);
  digitalWrite(LED_VERMELHO, LOW);
}

// ---------- TECLADO ----------
void verificarTeclado() {
  char tecla = teclado.getKey();
  if (!tecla) { return; }

  if (tecla == '#') {
    Serial.print("[Senha] ");
    if (digitado == SENHA_CORRETA) { acessoLiberado(); }
    else { acessoNegado(); }
    digitado = "";
  }
  else if (tecla == '*') {
    digitado = "";
    Serial.println("Digitacao apagada.");
  }
  else {
    digitado += tecla;
    Serial.print("Digitando... (");
    Serial.print(digitado.length());
    Serial.println(" teclas)");
  }
}

// ---------- DIGITAL ----------
void esperarTirarDedo() {
  unsigned long inicio = millis();
  delay(200);
  // espera o dedo sair, mas no MAXIMO 2 segundos
  while (leitor.getImage() != FINGERPRINT_NOFINGER) {
    if (millis() - inicio > 2000) { break; }   // desiste e segue
    delay(50);
  }
}

void verificarDigital() {
  if (!leitorOK) { return; }                              // sem leitor: ignora
  if (leitor.getImage() != FINGERPRINT_OK) { return; }    // sem dedo: sai

  if (leitor.image2Tz(1) != FINGERPRINT_OK) {
    Serial.println("[Digital] Imagem ruim, tente de novo.");
    esperarTirarDedo();
    return;
  }

  if (leitor.fingerFastSearch() == FINGERPRINT_OK) {      // busca rapida
    Serial.print("[Digital] ID ");
    Serial.print(leitor.fingerID);
    Serial.print(" - ");
    acessoLiberado();
  } else {
    Serial.print("[Digital] nao reconhecida - ");
    acessoNegado();
  }
  esperarTirarDedo();
}

// ---------- LOOP PRINCIPAL ----------
void loop() {
  batimento();            // pisca o LED da placa
  verificarTeclado();     // alguem apertou tecla?
  verificarDigital();     // alguem colocou o dedo?
}