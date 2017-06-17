#include <Stepper.h>
#include <LiquidCrystal.h>
#include <Adafruit_NeoPixel.h>

const int STEPS_PER_REVOLUTION = 500;
const int PASSO_MOTOR = 80;

Stepper motor(STEPS_PER_REVOLUTION, 7, 9, 8, 10);
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Adafruit_NeoPixel neoPixel = Adafruit_NeoPixel(8, 13, NEO_GRB + NEO_KHZ800);

//Definição das frequencias em Hz
const double E6 = 82;
const double L6 = 55;   //Usamos L == A(lá) devido ao conflito com as portas analógicas do arduino(A0,A1,A2,A3,...)
const double Bb6 = 58.3;
const double B6 = 61.8;
const double C6 = 65.5;
const double Db6 = 69.4;
const double D6 = 73.5;
const double Eb6 = 77.8;

const double L5 = 110;
const double F5 = 87.3;
const double Gb5 = 92.5;
const double G5 = 98;
const double Lb5 = 103.8;

const double D4 = 146.8;
const double Bb4 = 116.5;
const double B4 = 123.4;
const double C4 = 130.8;
const double Db4 = 138.5;

const double G3 = 196;
const double Eb3 = 155.5;
const double E3 = 164.8;
const double F3 = 174.6;
const double Gb3 = 185;

const double B2 = 247;
const double Lb2 = 207.7;
const double L2 = 220;
const double Bb2 = 233;

const double E1 = 329.6;
const double C1 = 261.6;
const double Db1 = 277.15;
const double D1 = 293.6;
const double Eb1 = 311.1;

//Agrupamento de frequencias por corda
double freqCorda6[8] = {L6, Bb6, B6, C6, Db6, D6, Eb6, E6};
double freqCorda5[8] = {D6, Eb6, E6, F5, Gb5, G5, Lb5, L5};
double freqCorda4[8] = {G5, Lb5, L5, Bb4, B4, C4, Db4, D4};
double freqCorda3[8] = {C4, Db4, D4, Eb3, E3, F3, Gb3, G3};
double freqCorda2[8] = {E3, F3, Gb3, G3, Lb2, L2, Bb2, B2};
double freqCorda1[8] = {L2, Bb2, B2, C1, Db1, D1, Eb1, E1};

//Agrupamento das representações das notas por corda
char* Corda6[8] = {"A ", "A#", "B ", "C ", "C#", "D ", "D#", "E "};
char* Corda5[8] = {"D ", "D#", "E ", "F ", "F#", "G ", "G#", "A "};
char* Corda4[8] = {"G ", "G#", "A ", "A#", "B ", "C ", "C#", "D "};
char* Corda3[8] = {"C ", "C#", "D ", "D#", "E ", "F ", "F#", "G "};
char* Corda2[8] = {"E ", "F ", "F#", "G ", "G#", "A ", "A#", "B "};
char* Corda1[8] = {"A ", "A#", "B ", "C ", "C#", "D ", "D#", "E "};

int bC[6] = {7, 7, 7, 7, 7, 7};
double freqBaseCorda[6] = {freqCorda1[bC[0]], freqCorda2[bC[1]], freqCorda3[bC[2]], freqCorda4[bC[3]], freqCorda5[bC[4]], freqCorda6[bC[5]]};

//Variáveis auxiliares para os arrays
int k = 5; //Utilizada durante a etapa de setup das frequências desejadas
int cordaEmAfinacao = 5;

//Portas
const int BTN_INDICADOR = A1, BTN_TROCA_TOM = A2, BTN_TROCA_CORDA = A3, BTN_PLAY_RESET = 6, ENTRADA_SINAL = A5; 

//Variáveis de estado
bool erro = false;
bool trocandoCorda = false;
bool comecouAfinacao = false;

const double MARGEM_DE_ERRO_AFINACAO = 0.3;

//Variável que guarda um valor de frequência utilizada para evitar que o usuário prejudique seu instrumento tocando a corda que não está sob o processo de afinação
double freqSeg = 1;   

//Cores para os leds
const uint32_t VERMELHO = neoPixel.Color(200, 0, 0);
const uint32_t VERDE = neoPixel.Color(0, 200, 0);
const uint32_t AZUL = neoPixel.Color(0, 0, 200);
const uint32_t AMARELO = neoPixel.Color(200, 150, 0);
const uint32_t INCOLOR = neoPixel.Color(0, 0, 0);

const int LED_ERRO = 6;

void setup() {
  Serial.begin(9600);
  pinMode(BTN_INDICADOR, INPUT);
  pinMode(BTN_TROCA_TOM, INPUT);
  pinMode(BTN_TROCA_CORDA, INPUT);
  pinMode(ENTRADA_SINAL, INPUT);
  pinMode(BTN_PLAY_RESET , INPUT);
  neoPixel.begin();
  neoPixel.setPixelColor(7, AZUL);
  neoPixel.show();
  lcd.begin(16, 2);
  motor.setSpeed(60);
}

double mediaAritimetica(long valor1, long valor2) {
  return (valor1 + valor2) / 2;
}

double leFrequencia() {
  int amplitudeInicial = 0;
  int amplitudeAtual = 0;
  int amplitudeAnterior = 0; 
  int amplitudeBaseMax = 201;
  int amplitudeBaseMin = 200;
  
  //As variáveis de tempo têm seus valores em microssegundos
  long instanteAtual = 0;
  long instanteInicioSemiPeriodo = 0;
  long semiPeriodo1 = 0;
  long semiPeriodo2 = 0;
  long disparidadeMaxima = 100; //Diferença máxima aceitável entre os semiperíodos encontrados 
  
  bool leuAmplitudeInicial = false;
  bool comecouLeitura = false;

  double frequencia = 0;
  
  while(!frequencia) {
    instanteAtual = micros();
    amplitudeAtual = analogRead(ENTRADA_SINAL);
    
    if(amplitudeAtual > amplitudeBaseMax)
      amplitudeAtual = 1023;
    else if(amplitudeAtual < amplitudeBaseMin)
      amplitudeAtual = 0;
      
    if(comecouLeitura) {
      if(amplitudeAtual != amplitudeAnterior) {
        if(!semiPeriodo1) {
          semiPeriodo1 = instanteAtual - instanteInicioSemiPeriodo;
          instanteInicioSemiPeriodo = instanteAtual;
        } else if(!semiPeriodo2) {
          semiPeriodo2 = instanteAtual - instanteInicioSemiPeriodo;
        }
        if(semiPeriodo2) {
          long diferencaSemiPeriodos = semiPeriodo2 - semiPeriodo1;
          if(diferencaSemiPeriodos > disparidadeMaxima || diferencaSemiPeriodos < (0 - disparidadeMaxima)) { //Elimina leituras que estão fora dos parâmetros desejados
            semiPeriodo1 = 0;
            semiPeriodo2 = 0;
          } else {
            double periodo = 2 * mediaAritimetica(semiPeriodo1, semiPeriodo2);
            frequencia = 1 / (periodo * pow(10, -6));
          }
        }
      }
      amplitudeAnterior = amplitudeAtual;
    } else {
      if(!leuAmplitudeInicial) {
        amplitudeInicial = amplitudeAtual;
        leuAmplitudeInicial = true;
      }
      if(amplitudeAtual != amplitudeInicial && leuAmplitudeInicial) {
        comecouLeitura = true;
        amplitudeAnterior = amplitudeAtual;
        instanteInicioSemiPeriodo = instanteAtual;
      }
    }
  }
  
  return frequencia;
}

double encontraFrequenciaMedia(){
  const int QTD_AMOSTRAS = 15; 
  double freqAmostradas[QTD_AMOSTRAS];
  double somaFrequenciasNaMedia = 0;
  int tamanhoArrayMedia = 0;
  int numFreqForaCurva = 0;
  double frequenciaMedia = 0;
  int indice = 0;// indice do 2 array, o array das medias, que só será declarado mais para frente no código.
  double comparacao = 0;
  double parametro = 0;
  
  for(int i = 0; i < QTD_AMOSTRAS; i++){
    freqAmostradas[i] = leFrequencia();
  }
  for(int i = 0; i < QTD_AMOSTRAS; i++){
    comparacao = freqAmostradas[i] - freqAmostradas[i + 1];
    if(abs(comparacao) < 10){
      parametro = (freqAmostradas[i] + freqAmostradas[i + 1])/2;
      break;
    }  
  }
  for(int i = 0; i < QTD_AMOSTRAS; i++){
    if(freqAmostradas[i] < parametro - 5 || freqAmostradas[i] > parametro + 5){
      numFreqForaCurva = numFreqForaCurva + 1;
    }
  }
  tamanhoArrayMedia = QTD_AMOSTRAS - numFreqForaCurva;
  double freqNaMedia[tamanhoArrayMedia];
  for(int i = 0; i < QTD_AMOSTRAS; i++){
    if(freqAmostradas[i] < parametro + 5 && freqAmostradas[i] > parametro - 5){
      freqNaMedia[indice] = freqAmostradas[i];
      indice++;
    }
  }
  for(int i = 0; i < tamanhoArrayMedia; i++){
      somaFrequenciasNaMedia = somaFrequenciasNaMedia + freqNaMedia[i]; 
  }
  frequenciaMedia = somaFrequenciasNaMedia/tamanhoArrayMedia;
  
  return frequenciaMedia;
}

double frequenciaFinalLida(){
  const int QTD_AMOSTRAS = 8; 
  double mediasAmostradas[QTD_AMOSTRAS];
  double somaFrequenciasMedias = 0;
  int tamanhoArrayMedias = 0;
  int numFreqMediasForaCurva = 0;
  double frequenciaFinal = 0;
  int indice = 0;// indice do 2 array, o array das medias, que só será declarado mais para frente no código.
  double comparacao = 0;
  double parametro = 0;
  
  for(int i = 0; i < QTD_AMOSTRAS; i++){
    mediasAmostradas[i] = encontraFrequenciaMedia();
  }
  for(int i = 0; i < QTD_AMOSTRAS; i++){
    comparacao = mediasAmostradas[i] - mediasAmostradas[i + 1];
    if(abs(comparacao) < 5){
      parametro = (mediasAmostradas[i] + mediasAmostradas[i + 1])/2;
      break;
    }  
  }
  for(int i = 0; i < QTD_AMOSTRAS; i++){
    if(mediasAmostradas[i] < parametro - 1 || mediasAmostradas[i] > parametro + 1){
      numFreqMediasForaCurva = numFreqMediasForaCurva + 1;
    }
  }
  tamanhoArrayMedias = QTD_AMOSTRAS - numFreqMediasForaCurva;
  double freqMediaNaMedia[tamanhoArrayMedias];
  for(int i = 0; i < QTD_AMOSTRAS; i++){
    if(mediasAmostradas[i] < parametro + 1 && mediasAmostradas[i] > parametro - 1){
      freqMediaNaMedia[indice] = mediasAmostradas[i];
      indice++;
    }
  }
  for(int i = 0; i < tamanhoArrayMedias; i++){
      somaFrequenciasMedias = somaFrequenciasMedias + freqMediaNaMedia[i]; 
  }
  frequenciaFinal = somaFrequenciasMedias/tamanhoArrayMedias;
  
  return frequenciaFinal;
}

void display() {
  //Programação do display
  lcd.print(Corda6[bC[5]]);
  lcd.setCursor(7, 0);
  lcd.print(Corda5[bC[4]]);
  lcd.setCursor(14, 0);
  lcd.print(Corda4[bC[3]]);
  lcd.home();
  lcd.setCursor(0, 1);
  lcd.print(Corda3[bC[2]]);
  lcd.setCursor(7, 1);
  lcd.print(Corda2[bC[1]]);
  lcd.setCursor(14, 1);
  lcd.print(Corda1[bC[0]]);
  lcd.home();

}

void verificaReset(int resetForcado) {
  if (digitalRead(BTN_PLAY_RESET) || resetForcado) {
    //Reseta as variáveis
    comecouAfinacao = false;
    erro = false;
    trocandoCorda = false;
    cordaEmAfinacao = 5;
    k = 5;
    freqSeg = 1;
    for (int i = 0; i < 6; i++) {
      bC[i] = 7;
      acendeLed(i, INCOLOR);
    }
    
    while (digitalRead(BTN_PLAY_RESET)); //Para o botão não registrar múltiplos clicks
    lcd.clear();
  }
}

void giraAntiHorario() {
  int passoAntiHorario = 0 - PASSO_MOTOR;
  motor.step(passoAntiHorario);
}

void giraHorario() {
  motor.step(PASSO_MOTOR);
}


void paraMotor() {
  motor.step(0);
}

void posicionaIndicador(int a, int b, int d, int f, int m, int n, char h, int* i) {
  if ((*i) == a) {
    lcd.setCursor(b, m);
    lcd.print(h);
    lcd.home();
    lcd.setCursor(d, 0);
    lcd.print(" ");
    lcd.home();
    lcd.setCursor(f, 0);
    lcd.print(" ");
    lcd.home();
    lcd.setCursor(b, n);
    lcd.print(" ");
    lcd.home();
    lcd.setCursor(d, 1);
    lcd.print(" ");
    lcd.home();
    lcd.setCursor(f, 1);
    lcd.print(" ");
    lcd.home();
  }
}

void botao(int porta, int* variavel, int lim1, int lim2) {
  int contador = 0;
  while (digitalRead(porta)) {
    delay(15);
    contador++;
  }
  if (contador <= 20 && contador != 0) {
    (*variavel)--;
  }
  if (contador > 20) {
    (*variavel)++;
  }
  if ((*variavel) == lim1) {
    (*variavel) = 0;
  }
  if ((*variavel) == -1) {
    (*variavel) = lim2;
  }
}

void acendeLed(int posicao, uint32_t cor) {
  neoPixel.setPixelColor(posicao, cor);
  neoPixel.show();
}

void loop() {
  if (comecouAfinacao == false) {
    botao(BTN_TROCA_TOM, &bC[k], 8, 7);
    botao(BTN_INDICADOR, &k, 6, 5);
    
    display();
    posicionaIndicador(5, 2, 6, 13, 0, 1, '<', &k);
    posicionaIndicador(4, 6, 2, 13, 0, 1, '>', &k);
    posicionaIndicador(3, 13, 6, 2, 0, 1, '>', &k);
    posicionaIndicador(2, 2, 6, 13, 1, 0, '<', &k);
    posicionaIndicador(1, 6, 2, 13, 1, 0, '>', &k);
    posicionaIndicador(0, 13, 6, 2, 1, 0, '>', &k);

    if (digitalRead(BTN_PLAY_RESET)) {
      while (digitalRead(BTN_PLAY_RESET));
      comecouAfinacao = true;
      acendeLed(cordaEmAfinacao, VERMELHO);
    }
  }

  if (comecouAfinacao == true) {
    if (erro == false) {
      double freqColetada = frequenciaFinalLida();
      Serial.println(freqColetada);
  
      if (freqSeg == freqColetada) {
        erro = true;
        lcd.clear();
        lcd.print("Erro!! Afinando");
        lcd.setCursor(0, 1);
        lcd.print("a corda errada");
        acendeLed(LED_ERRO, AMARELO);
      } else {
        freqSeg = freqColetada;
      }
      
      //Corda afinada
      if ((freqColetada >= freqBaseCorda[cordaEmAfinacao] - MARGEM_DE_ERRO_AFINACAO) && (freqColetada <= freqBaseCorda[cordaEmAfinacao] + MARGEM_DE_ERRO_AFINACAO)) { 
        paraMotor();
        acendeLed(cordaEmAfinacao, VERDE);
        trocandoCorda = true;
      }

      if(!trocandoCorda) {
        //Afrouxa a corda
        if (freqBaseCorda[cordaEmAfinacao] > freqColetada) giraAntiHorario();
  
        //Aperta a corda
        if (freqBaseCorda[cordaEmAfinacao] < freqColetada) giraHorario();
      }
      
      while(trocandoCorda){
        if(cordaEmAfinacao > 0) { //Verifica não está na última corda
          verificaReset(0);
          if(digitalRead(BTN_TROCA_CORDA)) {
            trocandoCorda = false;
            cordaEmAfinacao--;
            acendeLed(cordaEmAfinacao, VERMELHO);
            while(digitalRead(BTN_TROCA_CORDA));
          }
        } else {
          delay(1000);
          verificaReset(1);
        }
      }
    }
    
    verificaReset(0);
    
    if (erro == true) {
      if (digitalRead(BTN_TROCA_CORDA)) {
        erro = false;
        acendeLed(LED_ERRO, INCOLOR);
        
        lcd.clear();
        display();
        posicionaIndicador(5, 2, 6, 13, 0, 1, 'x', &cordaEmAfinacao);
        posicionaIndicador(4, 6, 2, 13, 0, 1, 'x', &cordaEmAfinacao);
        posicionaIndicador(3, 13, 6, 2, 0, 1, 'x', &cordaEmAfinacao);
        posicionaIndicador(2, 2, 6, 13, 1, 0, 'x', &cordaEmAfinacao);
        posicionaIndicador(1, 6, 2, 13, 1, 0, 'x', &cordaEmAfinacao);
        posicionaIndicador(0, 13, 6, 2, 1, 0, 'x', &cordaEmAfinacao);
        
        while (digitalRead(BTN_TROCA_CORDA));
      }
    }
  }
}
