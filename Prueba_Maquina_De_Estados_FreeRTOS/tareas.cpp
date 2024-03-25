/**
FreeRTOS tasks for reading sensors.
Calibration
@author Vadim Coselev
@author Asier Legaz
*/

#include "global_definitions.h"
#include "tareas.h"

bool DETECTOR_DE_CRUCE_ENABLE = false;
SemaphoreHandle_t semaforo_cruce = xSemaphoreCreateBinary();
SemaphoreHandle_t semaforo_IR_Puerta_Liberar_Puerto_27 = xSemaphoreCreateBinary();

#define COLOR 1
#define APARCAR 2
#define ZUMBADOR 3
#define ESPERA 4

  QTRSensors qtr;
  QTRSensors qtrRC;
  const uint8_t SensorCount = 4;
  uint16_t sensorValues[SensorCount];
  uint16_t RCsensorValues[2];

  const double kp = 0.085;
  const double kd = 0.003;
  const double ki = 0.002;
  const uint16_t calSpeed = 135;
  uint16_t baseSpeed = 85;

  int16_t error = 0;
  int16_t acumError = 0;
  int16_t lastError = 0;
  int16_t speedError = 0;
  const uint16_t speedErrorLIM = 100;  
  int16_t  velocidadA;
  int16_t  velocidadB;


  const uint8_t RC_I = 5;
  const uint8_t RC_D = 16;
  const uint16_t RCLimI= 450;
  const uint16_t RCLimD= 450;

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
      Serial.print(RCsensorValues[0]); Serial.print(", "); Serial.println(RCsensorValues[1]);
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
#define ADELANTE 1
#define ATRAS 
bool SENTIDO_MOVIMIENTO = ADELANTE;
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

      if(SENTIDO_MOVIMIENTO)
      {
          if(velocidadA >= 256){forwardA(255);}
          else if(velocidadA >= 0){forwardA(velocidadA);}
          else{backwardA(velocidadA);}

          if(velocidadB >= 256){forwardB(255);}
          else if(velocidadB >= 0){forwardB(velocidadB);}
          else{backwardB(velocidadB);}
      }
      else
      {
          if(velocidadA >= 256){backwardA(255);}
          else if(velocidadA >= 0){backwardA(velocidadA);}
          else{forwardA(velocidadA);}

          if(velocidadB >= 256){backwardB(255);}
          else if(velocidadB >= 0){backwardB(velocidadB);}
          else{forwardB(velocidadB);}
      }      
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
  if(angulo == 90) 
  {
    vTaskDelay(pdMS_TO_TICKS(250));
    backwardA(calSpeed+85); forwardB(calSpeed+95);
  }// Serial.println( " Giro a izquierda");}
  else if(angulo == -90) 
  {
    vTaskDelay(pdMS_TO_TICKS(150));
    forwardA(calSpeed+85); backwardB(calSpeed+95);
  }// Serial.println( " Giro a derecha");}
  vTaskDelay(pdMS_TO_TICKS(300));
  //uint16_t position;
  uint16_t position = qtr.readLineBlack(sensorValues);
  for(uint16_t k=0; k<2; k++){while(sensorValues[1] < 500 || sensorValues[2] < 500) {position = qtr.readLineBlack(sensorValues);}}
  /*do
  {
  position = qtr.readLineBlack(sensorValues);
  }while(position < 1350 || position > 1650);*/
  lastError = 0;
  acumError = 0;
  forwardA(0); backwardB(0);
  vTaskDelay(pdMS_TO_TICKS(500));
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

    else if (i < 175){ backwardA(calSpeed+50); forwardB(calSpeed+75);}    

    else if (i < 200){ backwardA(calSpeed); forwardB(calSpeed);}

    else if (i < 225){ backwardA(calSpeed+50); forwardB(calSpeed+75);}   

    else if (i < 250){ backwardA(calSpeed); forwardB(calSpeed);}

    else if (i < 275){ backwardA(calSpeed+50); forwardB(calSpeed+75);}    

    else if (i < 300){ forwardA(calSpeed); backwardB(calSpeed); }

    else if (i < 350){ forwardA(calSpeed+50); backwardB(calSpeed+75); }

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

void enviar_IR_puerta()
{
  uint16_t PIN_IR = 27;
  IRsend Emisor_IR(PIN_IR);
  Emisor_IR.begin();
  uint32_t data = Emisor_IR.encodeNEC(0x00,0x09);
  Serial.println(data,HEX);
  Emisor_IR.sendNEC(data);
}

bool CAMARA_TERMOGRAFICA_ENABLE = false;
bool PUNTO_CALIENTE = false;
bool MEDICION_CAMARA_REALIZADA = false;
void MLX90641_Read(void * pvParameters){

  uint16_t registro_inicial_RAM = 0x0440; // Quitamos las dos primeras filas
  uint16_t registro_final_RAM = 0x0540-1; //Las dos ultimas filas las quitamos
  uint8_t SDA_pin = 32;
  uint8_t SCL_pin = 33;
  uint32_t frequency_I2C = 100000;
  uint16_t ADDRESS_SLAVE = 0x33;

  static SuperI2C MLX90641(SDA_pin, SCL_pin, frequency_I2C, ADDRESS_SLAVE);

  MLX90641.Write_Register_I2C({0x24,0x0C},{0x00,0x00});
  vTaskDelay(pdMS_TO_TICKS(6));
  MLX90641.Write_Register_I2C({0x24,0x0C},{0x0B,0x81});
  vTaskDelay(pdMS_TO_TICKS(6));
  
  MLX90641.Write_Register_I2C({0x80,0x0D},{0x00,0x00});
  vTaskDelay(pdMS_TO_TICKS(6));
  MLX90641.Write_Register_I2C({0x80,0x0D},{0x0B,0x81});
  vTaskDelay(pdMS_TO_TICKS(6));
  vector <byte> Config_RAM = MLX90641.Read_Register_I2C({0x80,0x0D}, 2);
  vTaskDelay(pdMS_TO_TICKS(200));//MS To wait config MLX90641
  
  while(1){
    if(CAMARA_TERMOGRAFICA_ENABLE == true)
    {
      uint8_t contador_linea;
      int16_t diferencias_acumuladas;
      uint8_t i_value_minus_1;

      contador_linea = 0;
      diferencias_acumuladas = 0;
      vector<uint8_t> Status_Register = MLX90641.Read_Register_I2C({0x80,0x00}, 2);
      while(Status_Register[1]>>7)
      {
        Status_Register = MLX90641.Read_Register_I2C({0x80,0x00}, 2);
      }
      for(uint16_t r_i = registro_inicial_RAM+2; r_i < registro_final_RAM+1; r_i = r_i + 0x0001)
      {
        vector<uint8_t> i_value = MLX90641.Read_Register_I2C({r_i>>8 & 0x00FF,r_i & 0x00FF}, 2);
        diferencias_acumuladas = diferencias_acumuladas + abs(i_value[1] - i_value_minus_1);
        contador_linea = contador_linea +1;
        if(contador_linea == 32){
          contador_linea = 0;
          r_i = r_i + 32;
        }
        i_value_minus_1 = i_value[1];
      }
      MLX90641.Write_Register_I2C({0x80,0x00},{0x11,B11101111});
      if(diferencias_acumuladas > 3500){
        PUNTO_CALIENTE = true;
      }
      else{
        PUNTO_CALIENTE = false;
      }
      MEDICION_CAMARA_REALIZADA = true;

      contador_linea = 0;
      diferencias_acumuladas = 0;
      Status_Register = MLX90641.Read_Register_I2C({0x80,0x00}, 2);
      while(Status_Register[1]>>7)
      {
        Status_Register = MLX90641.Read_Register_I2C({0x80,0x00}, 2);
      }
      
      for(uint16_t r_i = registro_inicial_RAM+34; r_i < registro_final_RAM+1; r_i = r_i + 0x0001)
      {
        vector<uint8_t> i_value = MLX90641.Read_Register_I2C({r_i>>8 & 0x00FF, r_i & 0x00FF}, 2);
        diferencias_acumuladas = diferencias_acumuladas + abs(i_value[1] - i_value_minus_1);
        contador_linea = contador_linea + 1;
        if(contador_linea == 32){
          contador_linea = 0;
          r_i = r_i + 32;
      }
      i_value_minus_1 = i_value[1];
    
      }
      if(diferencias_acumuladas > 3500){
        PUNTO_CALIENTE = true;
      }
      else{
        PUNTO_CALIENTE = false;
      }
      MEDICION_CAMARA_REALIZADA = true;
        
      MLX90641.Write_Register_I2C({0x80,0x00},{0x11,B11101111});
      }
    else
    {
      vTaskDelay(pdMS_TO_TICKS(20));
    }
  }
}

/*
bool LASER_ENABLE = false;
void vLaser_Read(void * pvParameters)
{
  uint8_t SDA_pin = 32;
  uint8_t SCL_pin = 33;
  uint16_t frequency_I2C = 100000;
  uint8_t ADDRESS_SLAVE = 0x29;

  //..................hay que crear un objeto global o configurar todos los registros
  SuperI2C laser(SDA_pin, SCL_pin, frequency_I2C, ADDRESS_SLAVE);
  //.............configuracion de registros

  if (laser.Read_Register_I2C({0x00, 0x00}, 1)[0] != 0xB4) {
    vTaskDelay(pdMS_TO_TICKS(500));
  }
  else 
  {
  }  
  if ((laser.Read_Register_I2C({0x00, 0x16}, 1)[0] && 0x01) == 1){
    laser.Write_Register_I2C({0x02, 0x07}, {0x01});
    laser.Write_Register_I2C({0x02, 0x08}, {0x01});
    laser.Write_Register_I2C({0x00, 0x96}, {0x00});
    laser.Write_Register_I2C({0x00, 0x97}, {0xFD});
    laser.Write_Register_I2C({0x00, 0xE3}, {0x00});
    laser.Write_Register_I2C({0x00, 0xE4}, {0x04});
    laser.Write_Register_I2C({0x00, 0xE5}, {0x02});
    laser.Write_Register_I2C({0x00, 0xE6}, {0x01});
    laser.Write_Register_I2C({0x00, 0xE7}, {0x03});
    laser.Write_Register_I2C({0x00, 0xF5}, {0x02});
    laser.Write_Register_I2C({0x00, 0xD9}, {0x05});
    laser.Write_Register_I2C({0x00, 0xDB}, {0xCE});
    laser.Write_Register_I2C({0x00, 0xDC}, {0x03});
    laser.Write_Register_I2C({0x00, 0xDD}, {0xF8});
    laser.Write_Register_I2C({0x00, 0x9F}, {0x00});
    laser.Write_Register_I2C({0x00, 0xA3}, {0x3C});
    laser.Write_Register_I2C({0x00, 0xB7}, {0x00});
    laser.Write_Register_I2C({0x00, 0xBB}, {0x3C});
    laser.Write_Register_I2C({0x00, 0xB2}, {0x09});
    laser.Write_Register_I2C({0x00, 0xCA}, {0x09});
    laser.Write_Register_I2C({0x01, 0x98}, {0x01});
    laser.Write_Register_I2C({0x01, 0xB0}, {0x17});
    laser.Write_Register_I2C({0x01, 0xAD}, {0x00});
    laser.Write_Register_I2C({0x00, 0xFF}, {0x05});
    laser.Write_Register_I2C({0x01, 0x00}, {0x05});
    laser.Write_Register_I2C({0x01, 0x99}, {0x05});
    laser.Write_Register_I2C({0x01, 0xA6}, {0x1B});
    laser.Write_Register_I2C({0x01, 0xAC}, {0x3E});
    laser.Write_Register_I2C({0x01, 0xA7}, {0x1F});
    laser.Write_Register_I2C({0x00, 0x30}, {0x00});

    // Recommended: Public registers - See data sheet for more detail
    laser.Write_Register_I2C({0x00, 0x11}, {0x10}); // Enables polling for 'New Sample ready' when measurement completes
    laser.Write_Register_I2C({0x01, 0x0A}, {0x30}); // Set the averaging sample period (compromise between lower noise and increased execution time)
    laser.Write_Register_I2C({0x00, 0x3F}, {0x46}); // Sets the light and dark gain (upper nibble). Dark gain should not be changed.
    laser.Write_Register_I2C({0x00, 0x31}, {0xFF}); // sets the # of range measurements after which auto calibration of system is performed
    laser.Write_Register_I2C({0x00, 0x41}, {0x63}); // Set ALS integration time to 100ms
    laser.Write_Register_I2C({0x00, 0x2E}, {0x01}); // perform a single temperature calibration of the ranging sensor

    // Optional: Public registers - See data sheet for more detail
    laser.Write_Register_I2C({0x00, 0x4B}, {0x09}); // Set default ranging inter-measurement period to 100ms
    laser.Write_Register_I2C({0x00, 0x3E}, {0x31}); // Set default ALS inter-measurement period to 500ms
    laser.Write_Register_I2C({0x00, 0x14}, {0x24}); // Configures interrupt on 'New Sample Ready threshold event'

    laser.Write_Register_I2C({0x00, 0x16}, {0x00});
    vTaskDelay(pdMS_TO_TICKS(500));
    //....................................................
    
  while(true)
  if(LASER_ENABLE == true)
  {
  while(!(laser.Read_Register_I2C({0x00,0x4d},1)[0] && 0x01)){   //comprobación de que no hay errores para poder empezar la lectura
    vTaskDelay(pdMS_TO_TICKS(1));
  }
  laser.Write_Register_I2C({0x00,0x18},{0x01});    //modo de una sola lectura
  while(!laser.Read_Register_I2C({0x00,0x4f},0x01)[0] && 0x04){   //espero a que la nueva muestra se tome
    vTaskDelay(pdMS_TO_TICKS(1));
  }
  uint8_t dist =laser.Read_Register_I2C({0x00,0x62},1)[0];   //leo la medida (la da directamente en mm)
  laser.Write_Register_I2C({0x00,0x15},{0x07});   //reseteo por si ha hbido un error para poder tomar más medidas
  vTaskDelay(pdMS_TO_TICKS(1));
  }
  }
  else
  {
    vTaskDelay(pdMS_TO_TICKS(10));
  }

}
*/