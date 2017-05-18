#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//Definição das frequencias em Hz
const float E6 = 82.5;
const float L6 = 55;   //Usamos L == A(lá) devido ao conflito com as portas analógicas do arduino(A0,A1,A2,A3,...)
const float Bb6 = 58.3;
const float B6 = 61.8;
const float C6 = 65.5;
const float Db6 = 69.4;
const float D6 = 73.5;
const float Eb6 = 77.8;


const float L5 = 110;
const float F5 = 87.3;
const float Gb5 = 92.5;
const float G5 = 98;
const float Lb5 = 103.8;


const float D4 = 146.8;
const float Bb4 = 116.5;
const float B4 = 123.4;
const float C4 = 130.8;
const float Db4 = 138.5;


const float G3 = 196;
const float Eb3 = 155.5;
const float E3 = 164.8;
const float F3 = 174.6;
const float Gb3 = 185;


const float B2 = 247;
const float Lb2 = 207.7;
const float L2 = 220;
const float Bb2 = 233;


const float E1 = 329.6;
const float C1 = 261.6;
const float Db1 = 277.15;
const float D1 = 293.6;
const float Eb1 = 311.1;

//Valores iniciais das variaveis dos botoes, bC# == botao da Corda #
int bC6 = 7;
int bC5 = 7;
int bC4 = 7;
int bC3 = 7;
int bC2 = 7;
int bC1 = 7;

//Estado do setup, false -> definindo, true -> começando a afinar

bool estado = false;

//Agrupamento de frequencias por corda
int freqCorda6[8] = {L6, Bb6, B6, C6, Db6, D6, Eb6, E6};
int freqCorda5[8] = {D6, Eb6, E6, F5, Gb5, G5, Lb5, L5};
int freqCorda4[8] = {G5, Lb5, L5, Bb4, B4, C4, Db4, D4};
int freqCorda3[8] = {C4, Db4, D4, Eb3, E3, F3, Gb3, G3};
int freqCorda2[8] = {E3, F3, Gb3, G3, Lb2, L2, Bb2, B2};
int freqCorda1[8] = {L2, Bb2, B2, C1, Db1, D1, Eb1, E1};

//Agrupamento das representações das notas por corda
char* Corda6[8] = {"A ", "A#", "B ", "C ", "C#", "D ", "D#", "E "};
char* Corda5[8] = {"D ", "D#", "E ", "F ", "F#", "G ", "G#", "A "};
char* Corda4[8] = {"G ", "G#", "A ", "A#", "B ", "C ", "C#", "D "};
char* Corda3[8] = {"C ", "C#", "D ", "D#", "E ", "F ", "F#", "G "};
char* Corda2[8] = {"E ", "F ", "F#", "G ", "G#", "A ", "A#", "B "};
char* Corda1[8] = {"A ", "A#", "B ", "C ", "C#", "D ", "D#", "E "};

int v[6] = {bC1, bC2, bC3, bC4, bC5, bC6};

int k = 5;

const int bCor = A1, bAfi = A2;

int a, b;

void setup() {

  //Definindo as portas ligadas aos botoes como entradas
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(7 , INPUT);
  lcd.begin(16, 2);
  Serial.begin(9600);
}

void display() {

  //Programação do display
  lcd.print(Corda6[v[5]]);
  lcd.setCursor(7, 0);
  lcd.print(Corda5[v[4]]);
  lcd.setCursor(14, 0);
  lcd.print(Corda4[v[3]]);
  lcd.home();
  lcd.setCursor(0, 1);
  lcd.print(Corda3[v[2]]);
  lcd.setCursor(7, 1);
  lcd.print(Corda2[v[1]]);
  lcd.setCursor(14, 1);
  lcd.print(Corda1[v[0]]);
  lcd.home();

}

void verificaReset() {
  if (digitalRead(6)) {
    while (digitalRead(6)); //Para o botão não registrar múltiplos clicks
    estado = false;

    //Reseta as variáveis
    v[5] = 7;
    v[4] = 7;
    v[3] = 7;
    v[2] = 7;
    v[1] = 7;
    v[0] = 7;
  }
}

void sinal(int a, int b, int c, int l, int d, int f, int m, int n, char h) {

  if (k == a) {
    lcd.setCursor(b, m);
    lcd.print(h);
    lcd.home();
    lcd.setCursor(d, c);
    lcd.print(" ");
    lcd.home();
    lcd.setCursor(f, c);
    lcd.print(" ");
    lcd.home();
    lcd.setCursor(b, n);
    lcd.print(" ");
    lcd.home();
    lcd.setCursor(d, l);
    lcd.print(" ");
    lcd.home();
    lcd.setCursor(f, l);
    lcd.print(" ");
    lcd.home();
  }
}

void botao(int conta, int porta, int *variavel, int lim1, int lim2) {
  conta = 0;
  while (digitalRead(porta)) {
    delay(15);
    conta++;
  }
  if (conta <= 20 && conta != 0) {
    (*variavel)--;
  }
  if (conta > 20) {
    (*variavel)++;
  }

  if ((*variavel) == lim1) {
    (*variavel) = 0;
  }
  if ((*variavel) == -1) {
    (*variavel) = lim2;
  }

}

void loop() {

  Serial.print(k);


  if (estado == false) {
    if (digitalRead(6)) {
      while (digitalRead(6));
      estado = true;
    }
    display();
    sinal(5, 2, 0,  1, 6, 13, 0, 1, '<');
    sinal(4, 6, 0, 1, 2, 13, 0, 1, '>');
    sinal(3, 13, 0, 1, 6, 2, 0, 1, '>');
    sinal(2, 2, 0, 1, 6, 13, 1, 0, '<' );
    sinal(1, 6, 0, 1, 2, 13, 1, 0, '>');
    sinal(0, 13, 0, 1, 6, 2, 1, 0, '>');

    botao(a, bAfi, &v[k], 8, 7);
    botao(b, bCor, &k, 6, 5);
  }

  if (estado == true) {
    verificaReset();

    //começa a afinar
  }
}
