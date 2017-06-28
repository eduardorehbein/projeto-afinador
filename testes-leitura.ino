int ENTRADA_SINAL = A6

void setup() {
  pinMode(ENTRADA_SINAL, INPUT);
  Serial.begin(9600);
}

double leFrequencia(int referencia) {
  int amplitudeInicial = 0;
  int amplitudeAtual = 0;
  int amplitudeAnterior = 0;
  
  //As variáveis de tempo têm seus valores em microssegundos
  long instanteAtual = 0;
  long instanteInicioSemiPeriodo = 0;
  long semiPeriodo1 = 0;
  long semiPeriodo2 = 0;

  //Diferença máxima aceitável entre os semiperíodos encontrados
  const long DISPARIDADE_MAXIMA_SEMI_PERIODOS = 300;  
  
  bool leuAmplitudeInicial = false;
  bool comecouLeitura = false;

  double frequencia = 0;
  
  while(!frequencia) {
    instanteAtual = micros();
    amplitudeAtual = analogRead(ENTRADA_SINAL);
    
    if(amplitudeAtual >= referencia)
      amplitudeAtual = 1;
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
            if(frequencia > 360) {
              frequencia = 0;
              semiPeriodo1 = 0;
              semiPeriodo2 = 0;
              instanteInicioSemiPeriodo = 0;
              amplitudeAnterior = 0;
              amplitudeInicial = 0;
              leuAmplitudeInicial = false;
              comecouLeitura = false;
            }
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

void testaReferencias() {
  referencia = 550;
  while(referencia < 800) {
    Serial.println(" ");
    Serial.print("Referencia: ");
    Serial.println(referencia);
    for(i = 0; i < 5; i++) {
      Serial.print(leFrequencia(referencia));
      if(i < 4) Serial.print(", ")
    }
    referencia++;
  }
}

void loop() {
  testaReferencias();
}
