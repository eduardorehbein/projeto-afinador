#include <Stepper.h>
#include <LiquidCrystal.h>
#include <Adafruit_NeoPixel.h>

//Variáveis relacionadas ao motor
const int STEPS_PER_REVOLUTION = 500;
const int PASSO_MOTOR = 80;
int girouAntiHorario = 0;
int girouHorario = 0;

//Objetos
Stepper motor(STEPS_PER_REVOLUTION, 7, 9, 8, 10);
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Adafruit_NeoPixel neoPixel = Adafruit_NeoPixel(8, 13, NEO_GRB + NEO_KHZ800);

//Definição das frequencias em Hz
const double E6 = 82;
const double L6 = 55; //Usamos L == A(lá) devido ao conflito com as portas analógicas do arduino(A0,A1,A2,A3,...)
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

//Array que guarda as posições do tom selecionado dentro dos arrays "CordaX" que foram declarados acima
int bC[6] = {7, 7, 7, 7, 7, 7};

//Array que guarda as frequências finais selecionadas para serem utilizadas como parâmetros de comparação durante a afinação
double freqBaseCorda[6];

//Variáveis auxiliares para os arrays
int cordaSelecionada = 5; //Utilizada durante a etapa de setup das frequências desejadas
int cordaEmAfinacao = 5;

//Portas de entrada de dados
const int BTN_INDICADOR = A1, BTN_TROCA_TOM = A2, BTN_TROCA_CORDA = A3, BTN_START_RESET = 6, ENTRADA_SINAL = A5; 

//Variáveis de estado
bool erro = false;
bool trocandoCorda = false;
bool comecouAfinacao = false;

//Variável que guarda um valor de frequência utilizada para evitar que o usuário prejudique seu instrumento tocando a corda que não está sob o processo de afinação
double freqSeg = 1;   

const double DISPARIDADE_MAXIMA_FREQ_SEG = 15;
const double MARGEM_DE_ERRO_AFINACAO = 0.3;

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
  pinMode(BTN_START_RESET , INPUT);
  neoPixel.begin();
  neoPixel.setPixelColor(7, AZUL);
  neoPixel.show();
  lcd.begin(16, 2);
  motor.setSpeed(60);
}

double leFrequencia() {
  int amplitudeInicial = 0;
  int amplitudeAtual = 0;
  int amplitudeAnterior = 0;
  int referencia = 200;
  
  //As variáveis de tempo têm seus valores em microssegundos
  long instanteAtual = 0;
  long instanteInicioSemiPeriodo = 0;
  long semiPeriodo1 = 0;
  long semiPeriodo2 = 0;

  //Diferença máxima aceitável entre os semiperíodos encontrados
  const long DISPARIDADE_MAXIMA_SEMI_PERIODOS = 100;  
  
  bool leuAmplitudeInicial = false;
  bool comecouLeitura = false;

  double frequencia = 0;
  
  while(!frequencia) {
    instanteAtual = micros();
    amplitudeAtual = analogRead(ENTRADA_SINAL);
    
    if(amplitudeAtual > referencia)
      amplitudeAtual = 1023;
    else if(amplitudeAtual < referencia)
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
          if(abs(diferencaSemiPeriodos) > DISPARIDADE_MAXIMA_SEMI_PERIODOS) { 
            //Elimina leituras que estão fora dos parâmetros desejados
            semiPeriodo1 = 0;
            semiPeriodo2 = 0;
            instanteInicioSemiPeriodo = 0;
            amplitudeAnterior = 0;
            amplitudeInicial = 0;
            leuAmplitudeInicial = false;
            comecouLeitura = false;
          } else {
            double periodo = semiPeriodo1 + semiPeriodo2;
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

double getFrequenciaMedia(){
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

void display() {
  //Programação do display
  lcd.print(Corda6[bC[5]]);
  lcd.setCursor(7, 0);
  lcd.print(Corda5[bC[4]]);
  lcd.setCursor(14, 0);
  lcd.print(Corda4[bC[3]]);
  lcd.setCursor(0, 1);
  lcd.print(Corda3[bC[2]]);
  lcd.setCursor(7, 1);
  lcd.print(Corda2[bC[1]]);
  lcd.setCursor(14, 1);
  lcd.print(Corda1[bC[0]]);
  lcd.home();
}

void posicionaIndicador(int a, int b, int d, int f, int m, int n, char h, int* i) {
  if ((*i) == a) {
    lcd.setCursor(b, m);
    lcd.print(h);
    lcd.setCursor(d, 0);
    lcd.print(" ");
    lcd.setCursor(f, 0);
    lcd.print(" ");
    lcd.setCursor(b, n);
    lcd.print(" ");
    lcd.setCursor(d, 1);
    lcd.print(" ");
    lcd.setCursor(f, 1);
    lcd.print(" ");
    lcd.home();
  }
}

void verificaReset(int resetForcado) {
  if (digitalRead(BTN_START_RESET) || resetForcado) {
    //Reseta as variáveis
    comecouAfinacao = false;
    erro = false;
    trocandoCorda = false;
    girouAntiHorario = 0; 
    girouHorario = 0;
    cordaEmAfinacao = 5;
    cordaSelecionada = 5;
    freqSeg = 1;
    for (int i = 0; i < 6; i++) {
      bC[i] = 7;
      acendeLed(i, INCOLOR);
    }
    
    while (digitalRead(BTN_START_RESET)); //Para o botão não registrar múltiplos clicks
    lcd.clear();
  }
}

void giraAntiHorario() {
  motor.step(0 - PASSO_MOTOR);
}

void giraHorario() {
  motor.step(PASSO_MOTOR);
}

void paraMotor() {
  motor.step(0);
}

void controlaVarComBtn(int portaBtn, int* variavel, int valorMaximo) {
  int contador = 0;
  while (digitalRead(portaBtn)) {
    delay(15);
    contador++;
  }
  if (contador <= 20 && contador != 0) {
    (*variavel)--;
  }
  if (contador > 20) {
    (*variavel)++;
  }
  if ((*variavel) == (valorMaximo + 1)) {
    (*variavel) = 0;
  }
  if ((*variavel) == -1) {
    (*variavel) = valorMaximo;
  }
}

void acendeLed(int posicao, uint32_t cor) {
  neoPixel.setPixelColor(posicao, cor);
  neoPixel.show();
}

void ativaErro() {
  erro = true;
  lcd.clear();
  lcd.print("Erro!! Afinando");
  lcd.setCursor(0, 1);
  lcd.print("a corda errada");
  acendeLed(LED_ERRO, AMARELO);
}

void verificaErro(double* freqColetada) {
  if(freqSeg != 1) {
    int nGiros = girouAntiHorario - girouHorario;
    double disparidadeFreq = freqSeg - (*freqColetada);
    
    if (abs(disparidadeFreq) > DISPARIDADE_MAXIMA_FREQ_SEG) { 
      //Se a diferença da frequência anterior para a atual é muito grande
      ativaErro();
    } else if(abs(nGiros) > 2 && (((*freqColetada) >= freqSeg - MARGEM_DE_ERRO_AFINACAO) && ((*freqColetada) <= freqSeg + MARGEM_DE_ERRO_AFINACAO))){
      //Se o motor está girando mas a frequência não está mudando
      ativaErro();
    } else {
      freqSeg = (*freqColetada);
    }
  } else {
    //Verifica se o usuário já começou a afinação da corda x tocando a corda errada
    giraAntiHorario();
    giraAntiHorario(); //Afrouxa
    double freqAnalise = getFrequenciaMedia();
    if((freqAnalise >= (*freqColetada) - MARGEM_DE_ERRO_AFINACAO) && (freqAnalise <= (*freqColetada) + MARGEM_DE_ERRO_AFINACAO)) {
      ativaErro();
    } else {
      (*freqColetada) = freqAnalise;
      freqSeg = (*freqColetada);
    }
  }
}

void loop() {
  if (comecouAfinacao == false) {
    controlaVarComBtn(BTN_TROCA_TOM, &bC[cordaSelecionada], 7);
    controlaVarComBtn(BTN_INDICADOR, &cordaSelecionada, 5);
    
    display();
    posicionaIndicador(5, 2, 6, 13, 0, 1, '<', &cordaSelecionada);
    posicionaIndicador(4, 6, 2, 13, 0, 1, '>', &cordaSelecionada);
    posicionaIndicador(3, 13, 6, 2, 0, 1, '>', &cordaSelecionada);
    posicionaIndicador(2, 2, 6, 13, 1, 0, '<', &cordaSelecionada);
    posicionaIndicador(1, 6, 2, 13, 1, 0, '>', &cordaSelecionada);
    posicionaIndicador(0, 13, 6, 2, 1, 0, '>', &cordaSelecionada);

    if (digitalRead(BTN_START_RESET)) {
      while (digitalRead(BTN_START_RESET));
      comecouAfinacao = true;
      acendeLed(cordaEmAfinacao, VERMELHO);
      freqBaseCorda[0] = freqCorda1[bC[0]]; 
      freqBaseCorda[1] = freqCorda2[bC[1]];
      freqBaseCorda[2] = freqCorda3[bC[2]];
      freqBaseCorda[3] = freqCorda4[bC[3]];
      freqBaseCorda[4] = freqCorda5[bC[4]];
      freqBaseCorda[5] = freqCorda6[bC[5]];
    }
  }

  if (comecouAfinacao == true) {
    verificaReset(0);
    
    if (erro == false) {
      double freqColetada = getFrequenciaMedia();
      Serial.println(freqColetada);

      //Verifica se o usuário está tocando uma corda que não está sendo afinada pelo equipamento no momento
      verificaErro(&freqColetada);
      
      //Corda afinada
      if ((freqColetada >= freqBaseCorda[cordaEmAfinacao] - MARGEM_DE_ERRO_AFINACAO) && (freqColetada <= freqBaseCorda[cordaEmAfinacao] + MARGEM_DE_ERRO_AFINACAO)) { 
        paraMotor();
        girouAntiHorario = 0; 
        girouHorario = 0;
        acendeLed(cordaEmAfinacao, VERDE);
        trocandoCorda = true;
      }

      if(!trocandoCorda) {
        //Afrouxa a corda
        if (freqBaseCorda[cordaEmAfinacao] > freqColetada) {
          giraAntiHorario();
          girouAntiHorario++;
        }
        //Aperta a corda
        if (freqBaseCorda[cordaEmAfinacao] < freqColetada) {
          giraHorario();
          girouHorario++;
        }
      }
      
      while(trocandoCorda){
        if(cordaEmAfinacao > 0) { //Verifica se já não está na última corda
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
