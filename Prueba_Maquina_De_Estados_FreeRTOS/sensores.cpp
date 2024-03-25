/**
FreeRTOS tasks for reading sensors.
Calibration
@author Vadim Coselev
@author Asier Legaz
*/


#include "global_definitions.h"
#include "sensor.h"

bool DETECTOR_DE_CRUCE_ENABLE = false;
SemaphoreHandle_t semaforo_cruce = xSemaphoreCreateBinary();

#define COLOR 1
#define APARCAR 2
#define ZUMBADOR 3
#define ESPERA 4

  QTRSensors qtr;
  QTRSensors qtrRC;
  const uint8_t SensorCount = 4;
  uint16_t sensorValues[SensorCount];
  uint16_t RCsensorValues[2];
  char sense[8] = {'F', 'F', 'F', 'R', 'F', 'F', 'F', 'F'};
  uint16_t senseIndex = 0;
  const double kp = 0.08;
  const double kd = 0.003;
  const double ki = 0.002;
  const uint16_t calSpeed = 70;
  const uint16_t baseSpeed = 75;
  int16_t error = 0;
  int16_t acumError = 0;
  int16_t lastError = 0;
  int16_t speedError = 0;
  const uint16_t speedErrorLIM = 100;  
  int16_t  velocidadA;
  int16_t  velocidadB;


  const uint8_t RC_I = 5;
  const uint8_t RC_D = 16;
  const uint16_t RCLimI= 600;
  const uint16_t RCLimD= 600;

  const uint8_t SEL0 = 27;
  const uint8_t SEL1 = 4;
  const uint8_t A_in = 34;
    // VARIABLES INICIALIZACION MOTORES
  const uint16_t PWMA = 25;         
  const uint16_t AIN2 = 17;        
  const uint16_t AIN1 = 21;         
  const uint16_t BIN1 = 22;       
  const uint16_t BIN2 = 23;        
  const uint16_t PWMB = 26;   

  const uint16_t ANALOG_WRITE_BITS = 8;

  int freq = 100000;
  int channel_A = 0;
  int channel_B = 1;
  int ADC_resolution = ANALOG_WRITE_BITS;


void vSimulador_Cruce(void * pvParameters)
{
  // configure the digital sensors
  qtrRC.setTypeRC();
  qtrRC.setSensorPins((const uint8_t[]){RC_I, RC_D}, 2);
  while(true)
  {
    if(DETECTOR_DE_CRUCE_ENABLE == true)
    { 
      //Ponemos que la va a detectar en un intervalo Random desde 500 ms a 2000 ms
      qtrRC.readLineBlack(RCsensorValues);
      if(RCsensorValues[0] > RCLimI || RCsensorValues[1] > RCLimD)
      {
        uint16_t sumL=RCsensorValues[0]; 
        uint16_t sumR=RCsensorValues[1];
        for(uint16_t j = 0; j<2; j++)
        {
          qtrRC.readLineBlack(RCsensorValues);

          sumL = sumL + RCsensorValues[0];
          sumR = sumR + RCsensorValues[1];
        }
        uint16_t avgL = sumL/3;
        uint16_t avgR = sumR/3;
        if(avgL>RCLimI || avgR>RCLimD)
        {
          //cruce();    // Función para girar al lado que toque
          xSemaphoreGive(semaforo_cruce);
          Serial.println("Cruce Detectado");
          DETECTOR_DE_CRUCE_ENABLE = false;
        }
      }
      vTaskDelay(pdMS_TO_TICKS(20));
    }
    else
    {
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  }
}

bool SEGUIR_LINEA_ENABLE = false;
void vSeguir_Linea(void * pvParameters)
{
  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){A_in, A_in, A_in, A_in}, SensorCount);
  qtr.setMultPins(SEL0, SEL1);
  while(true)
  {
    if(SEGUIR_LINEA_ENABLE == true)
    {
      uint16_t position = qtr.readLineBlack(sensorValues);
      error = 1500-position;

      acumError = error + acumError;
      if(error*acumError <0 ){acumError = 0;}

      speedError = kp * error + ki * (acumError) + kd * (error - lastError);
      
      lastError = error;

      velocidadA = baseSpeed - speedError;
      velocidadB = baseSpeed + speedError;

      if(velocidadA >= 256){forwardA(255);}
      else if(velocidadA >= 0){forwardA(velocidadA);}
      else{backwardA(velocidadA);}

      if(velocidadB >= 256){forwardB(255);}
      else if(velocidadB >= 0){forwardB(velocidadB);}
      else{backwardB(velocidadB);}
      vTaskDelay(pdMS_TO_TICKS(20));
    }
    else
    {
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  }
}


///////////////////////////////////
// INICIO FUNCIONES PARA MOTORES //
///////////////////////////////////
void initMotors(){
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

  ledcSetup(channel_A, freq, ADC_resolution);
  ledcAttachPin(PWMA, channel_A);

  ledcSetup(channel_B, freq, ADC_resolution);
  ledcAttachPin(PWMB, channel_B);
}

void forwardA(uint16_t pwm){
  digitalWrite(AIN2, LOW);
  digitalWrite(AIN1, HIGH);
  ledcWrite(channel_A, pwm);
}

void backwardA(uint16_t pwm){
  digitalWrite(AIN2, HIGH);
  digitalWrite(AIN1, LOW);
  ledcWrite(channel_A, pwm);
}

void forwardB(uint16_t pwm){
  digitalWrite(BIN2, LOW);
  digitalWrite(BIN1, HIGH);
  ledcWrite(channel_B, pwm);
}

void backwardB(uint16_t pwm){
  digitalWrite(BIN2, HIGH);
  digitalWrite(BIN1, LOW);
  ledcWrite(channel_B, pwm);
}
///////////////////////////////////
// FIN FUNCIONES PARA MOTORES //
///////////////////////////////////


///////////////////////////////////
// FUNCIONES PARA IR SIGUELINEAS //
///////////////////////////////////
void FUNCION_GIRO(int16_t angulo)
{
  if(angulo != 0)
  {
  vTaskDelay(pdMS_TO_TICKS(200));
  if(angulo == 90) {backwardA(calSpeed+70); forwardB(calSpeed+60);}// Serial.println( " Giro a izquierda");}
  else if(angulo == -90) {forwardA(calSpeed+70); backwardB(calSpeed+75);}// Serial.println( " Giro a derecha");}
  vTaskDelay(pdMS_TO_TICKS(1200));
  uint16_t position = qtr.readLineBlack(sensorValues);
  while(position < 1200 || position > 1800) {position = qtr.readLineBlack(sensorValues);}
  lastError = 0;
  acumError = 0;
  }
  else
  {}
}

void calibracion(void)
{
  // analogRead() takes about 0.1 ms on an AVR.
  // 0.1 ms per sensor * 4 samples per sensor read (default) * 4 sensors
  // * 10 reads per calibrate() call = ~16 ms per calibrate() call.
  // Call calibrate() 400 times to make calibration take about 10 seconds.
 for (uint16_t i = 0; i < 400; i=i+2)
  {
    if( i < 25){ forwardA(calSpeed); backwardB(calSpeed);}    

    else if (i < 50){ forwardA(calSpeed+50); backwardB(calSpeed+50);}

    else if( i < 75){ forwardA(calSpeed); backwardB(calSpeed);}    

    else if (i < 100){ forwardA(calSpeed+50); backwardB(calSpeed+50);}

    else if (i < 125){ backwardA(calSpeed); forwardB(calSpeed);}

    else if (i < 175){ backwardA(calSpeed+50); forwardB(calSpeed+50);}    

    else if (i < 200){ backwardA(calSpeed); forwardB(calSpeed);}

    else if (i < 225){ backwardA(calSpeed+50); forwardB(calSpeed+50);}   

    else if (i < 250){ backwardA(calSpeed); forwardB(calSpeed);}

    else if (i < 275){ backwardA(calSpeed+50); forwardB(calSpeed+50);}    

    else if (i < 300){ forwardA(calSpeed); backwardB(calSpeed); }

    else if (i < 350){ forwardA(calSpeed+50); backwardB(calSpeed+50); }

    else{ forwardA(calSpeed); backwardB(calSpeed);} 

    qtr.calibrate();
    qtrRC.calibrate();
  }
  forwardA(0); backwardB(0);
  

  for (uint8_t i = 0; i < SensorCount; i++)
  {
    Serial.print(qtr.calibrationOn.minimum[i]);
    Serial.print(' ');
  }

  Serial.println();

  // print the calibration maximum values measured when emitters were on
  for (uint8_t i = 0; i < SensorCount; i++)
  {
    Serial.print(qtr.calibrationOn.maximum[i]);
    Serial.print(' ');
  }
  Serial.println();

  for (uint8_t i = 0; i < 2; i++)
  {
    Serial.print(qtrRC.calibrationOn.minimum[i]);
    Serial.print(' ');
  }
  Serial.println();

  // print the calibration maximum values measured when emitters were on
  for (uint8_t i = 0; i < 2; i++)
  {
    Serial.print(qtrRC.calibrationOn.maximum[i]);
    Serial.print(' ');
  }
  Serial.println();

}

///////////////////////////////////
// FUNCIONES PARA IR SIGUELINEAS //
///////////////////////////////////


/////////////////////////////////
// FUNCIONES PARA COMUNICACIÓN //
/////////////////////////////////


/////////////////////////////////
// FUNCIONES PARA COMUNICACIÓN //
/////////////////////////////////