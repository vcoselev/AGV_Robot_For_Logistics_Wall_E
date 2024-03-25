/**
State Machine for Movements.
@author Vadim Coselev
@author Asier Legaz
*/

#include "Maquina_De_Estados_Movimiento.h"
#include "SCServo.h"

SMS_STS st;
#define S_RXD 18
#define S_TXD 19
//Iniciamos la maquina en el estado 1
uint8_t STATE_Movement_and_Functions = 1;
uint8_t WIFI_NODOS_ESPECIALES_SELECCION=0;

//Definimos la estructura para poder recibir el vector de Dijkstra
struct AVector received_struct_vector;
std::vector<int16_t> recived_vector;

TaskHandle_t TEST_SPECIAL_FUNCTION_Handle = NULL;
bool TEST_SPECIAL_FUNCTION_TASK_ENABLE = false;


bool GIRO_FALSE_FORWARD_TRUE = true; //True para forward. En la primera iteracion de Desplazar hacia la izquierda se podra en FALSE y por tanto empezamos por el GIRO.

//Estados del babyESP
uint8_t ABRIR_PUERTA_IF = 1;
uint8_t ACTIVAR_ZUMBADOR = 2;

void vTaskState_Machine_For_Movement_and_Functions(void * pvParameters)
{
    Serial1.begin(1000000, SERIAL_8N1, S_RXD, S_TXD);
    Serial1.begin(1000000, SERIAL_8N1, S_RXD, S_TXD);
    st.pSerial = &Serial1;

    /*
    0 -> ALL_SUSPENDED
    1 -> IDLE_MOVEMENT_AND_FUNCTIONS
    2 -> ARRAY_SHIFT
    3 -> GIRO
    4 -> FORWARD
    5 -> TEST_SPECIAL_FUNCTION
    */
    while (true)
    {
      BaseType_t  STATE_Machine_For_Movement_and_Functions_Trigger;
      STATE_Machine_For_Movement_and_Functions_Trigger = xQueueReceive(xQueue_STATE_Movement_and_Functions, &STATE_Movement_and_Functions, 0);
      if(STATE_Machine_For_Movement_and_Functions_Trigger == pdPASS)
      {
        switch (STATE_Movement_and_Functions)
        {
          case RECEIVE_ARRAY_STATE: //RECEIVE_ARRAY
              GIRO_FALSE_FORWARD_TRUE = true;
              recived_vector.clear();
              if(Queue_Handle_Vector_Dijkstra != NULL)
                      {
                        if(xQueueReceive(Queue_Handle_Vector_Dijkstra, 
                                          &(received_struct_vector), 
                                          (TickType_t) 10)==pdPASS)
                                          {
                                            Serial.println("Vector Dijkstra Recibido Correctamente");
                                          }
                      }
                      
              recived_vector.assign(received_struct_vector.vector_dijkstra.begin(), received_struct_vector.vector_dijkstra.end());
              //Serial.println(sizeof(recived_vector));
              for (int16_t element : recived_vector) 
              {
              //Imprimir cada elemento del vector
              Serial.println(element);
              }
              STATE_Movement_and_Functions = ARRAY_SHIFT_STATE;
              xQueueSend(xQueue_STATE_Movement_and_Functions, &STATE_Movement_and_Functions,0);

              break;
          case IDLE_MOVEMENT_AND_FUNCTIONS_STATE: //IDLE
              Serial.println("ESPERANDO COMANDO...");
              //STATE_Movement_and_Functions = ARRAY_SHIFT_STATE;
              //xQueueSend(xQueue_STATE_Movement_and_Functions, &STATE_Movement_and_Functions,0);
              DETECTOR_DE_CRUCE_ENABLE = false;
              SEGUIR_LINEA_ENABLE = false;
              forwardA(0);forwardB(0);
              STATE_Main_WiFi_ISR = IDLE_WIFI_MAIN;
              xQueueSend(xQueue_STATE_Main_WiFi_ISR, (void *)&STATE_Main_WiFi_ISR, 0);
              break;
          case ARRAY_SHIFT_STATE: //ARRAY SHIFT
              Serial.println("ARRAY SHIFT");
              recived_vector.erase(recived_vector.begin());
              if(recived_vector.empty()==0)
              {

                //Serial.println(recived_vector[0]);
                GIRO_FALSE_FORWARD_TRUE  = !GIRO_FALSE_FORWARD_TRUE;
                STATE_Movement_and_Functions = GIRO_FALSE_FORWARD_TRUE==true? FORWARD_STATE:GIRO_STATE;
                xQueueSend(xQueue_STATE_Movement_and_Functions, &STATE_Movement_and_Functions,0);
              }
              else
              {
                STATE_Movement_and_Functions = IDLE_MOVEMENT_AND_FUNCTIONS_STATE;
                xQueueSend(xQueue_STATE_Movement_and_Functions, &STATE_Movement_and_Functions,0);
              }
              break;
          case GIRO_STATE:
              //ACTUALIZO EL ESTADO PARA TENERLO NO PARA GIRAR
              if(recived_vector[0] != 0)
              {
              SEGUIR_LINEA_ENABLE = false;
              DETECTOR_DE_CRUCE_ENABLE = false;
              theta_state = theta_state + recived_vector[0];
              if(theta_state < 0){theta_state = theta_state +360;}
              theta_state = theta_state % 360; // Para dejarlo en un rango de 0 a 360.
              if( recived_vector[0] == 180 || recived_vector[0] == -180 ){
                Serial.println("COMIENZO GIRO...");
                FUNCION_GIRO(90);FUNCION_GIRO(90);
                Serial.println("FIN GIRO...");
              }
              else
              {
                Serial.println("COMIENZO GIRO...");
                FUNCION_GIRO(recived_vector[0]);
                Serial.println("FIN GIRO...");
              }
              
              //PARA GIRAR CON recived_vector[0] me vale
              Serial.print("NODO: "); Serial.println(node_state);
              Serial.print("ORIENTACIÓN: "); Serial.print(theta_state); Serial.println("[º]");
              STATE_Movement_and_Functions = TEST_SPECIAL_FUNCTION_STATE;
              }
              else{
                SEGUIR_LINEA_ENABLE = true;
                DETECTOR_DE_CRUCE_ENABLE = false;
                
                vTaskDelay(pdMS_TO_TICKS(500));
                STATE_Movement_and_Functions = ARRAY_SHIFT_STATE;
              }
              
              xQueueSend(xQueue_STATE_Movement_and_Functions, &STATE_Movement_and_Functions,0);
              break;

          case FORWARD_STATE:
              Serial.println("ENCENDEMOS EL DETECTOR DE CRUCE");
              SEGUIR_LINEA_ENABLE = true;
              vTaskDelay(pdMS_TO_TICKS(375));
              DETECTOR_DE_CRUCE_ENABLE = true;
              //Aqui encendemos tambien el sigue lineas para ir hacia delante.
              if (xSemaphoreTake(semaforo_cruce, portMAX_DELAY) == pdTRUE)
              {
              Serial.println("CRUCE DETECTADO");
              }
              node_state = recived_vector[0];
              Serial.print("NODO: "); Serial.println(node_state);
              Serial.print("ORIENTACIÓN: "); Serial.print(theta_state); Serial.println("[º]");
              STATE_Movement_and_Functions = TEST_SPECIAL_FUNCTION_STATE;
              xQueueSend(xQueue_STATE_Movement_and_Functions, &STATE_Movement_and_Functions,0);

              break;
          case TEST_SPECIAL_FUNCTION_STATE:
              Serial.println("Estamos comprobando si el nodo es especial");
              if(WIFI_NODOS_ESPECIALES_SELECCION){nodos_especial_vuelta_reconocimiento(node_state, theta_state);}
              else{nodos_especial_recogida_entrega(node_state, theta_state);}
              STATE_Movement_and_Functions = ARRAY_SHIFT_STATE;
              xQueueSend(xQueue_STATE_Movement_and_Functions, &STATE_Movement_and_Functions,0);
            break;
              break;
          default:
              break;
        }
      }
      else
      {
        vTaskDelay(pdMS_TO_TICKS(10));
      }
    }

}

void nodos_especial_vuelta_reconocimiento(uint8_t nodo, int16_t angulo)
{
  switch (nodo) 
  {
    case NODO_0:

        break;
    case NODO_1:
        // Código para el nodo 1
        break;
    case NODO_2:
        if(angulo == 0 || angulo == 180 || angulo == 360)
        {              
          
            
          detectar_punto_caliente();
        }
        break;
    case NODO_3:
        if(angulo == 0 || angulo == 180 || angulo == 360)
        {
          
            
          detectar_punto_caliente();
        }
        break;
    case NODO_4:
        if(angulo == 0 || angulo == 180 || angulo == 360)
        {         
          detectar_punto_caliente();
        }
        break;
    case NODO_5:
        // Código para el nodo 5
        break;
    case NODO_6:
        // Código para el nodo 6
        break;
    case NODO_7:
        // Código para el nodo 7
        break;
    case NODO_8:
        // Código para el nodo 8
        break;
    case NODO_9:
        // Código para el nodo 9
        break;
    case NODO_10:
        // Código para el nodo 10
        break;
    case NODO_11:
        break;
    case NODO_12:
        // Código para el nodo 12
        break;
    case NODO_13:
        // Código para el nodo 13
        break;
    case NODO_14:
        // Código para el nodo 14
        break;
    case NODO_15:
        if(angulo == 90)
        { 
          vTaskDelay(pdMS_TO_TICKS(200));
          deshabilitar_tareas_para_nodo_especial();
          Serial.println("Preparando WALL-E para pasar por la puerta.");
          Serial.println("Encendemos el IR para abrir la puerta");
          for(int i=0; i<16; i++){ACCION_BABY_ESP(ABRIR_PUERTA_IF); vTaskDelay(pdMS_TO_TICKS(250));}
          baseSpeed = 125; // Para que pase la puerta
          
          vTaskDelay(pdMS_TO_TICKS(1500));
          baseSpeed = 85;
        }
        break;
    case NODO_16:
        // Código para el nodo 16
        break;
    case NODO_17:
        // Código para el nodo 17
        break;
    case NODO_18:
      baseSpeed = 85;
      if(angulo == 0 || angulo == 180 || angulo == 360)
        {
            
          detectar_punto_caliente();
        }
        break;
    case NODO_19:
        // Código para el nodo 19
        break;
    case NODO_20:
      if(angulo == 0 || angulo == 180)
        {
            
          detectar_punto_caliente();
        }
        break;
    case NODO_21:
        // Código para el nodo 21
        break;
    default:
        // Código para manejar otros casos si es necesario
        break;
  }
}

void nodos_especial_recogida_entrega(uint8_t nodo, int16_t angulo)
{
      switch (nodo) 
    {
      case NODO_0:

          break;
      case NODO_1:
          // Código para el nodo 1
          break;
      case NODO_2:
        if(angulo == 0 || angulo == 180 || angulo == 360)
          {
              
            detectar_punto_caliente();
          }
        if(NODO_FINAL==7)
        {
          if(angulo == 90)
          {
            entrega_palet();
            //reiniciar dijkstra
          }
        }
        break;
      case NODO_3:
        if(angulo == 0 || angulo == 180 || angulo == 360)
          {
            detectar_punto_caliente();
          }
        if(NODO_FINAL==8)
        {
          if(angulo == 90)
          {
            entrega_palet();
            //reiniciar dijkstra
          }
        }
        break;
      case NODO_4:
        if(angulo == 0 || angulo == 180 || angulo == 360)
        {
          
            
          detectar_punto_caliente();
        }
        if(NODO_FINAL==9)
        {
          if(angulo == 90)
          {
            entrega_palet();
            //reiniciar dijkstra
          }
        }
        break;
      case NODO_5:
          // Código para el nodo 5
          if(angulo == 0 || angulo == 180 || angulo == 360)
          {
            
              
            detectar_punto_caliente();
          }
          if(NODO_FINAL==6)
          {
            if(angulo == 180)
              {
                recogida_palet();
                //reiniciar dijkstra
              }
          }
          break;
      case NODO_6:
          // Código para el nodo 6
          break;
      case NODO_7:
          // Código para el nodo 7
          break;
      case NODO_8:
          // Código para el nodo 8
          break;
      case NODO_9:
          // Código para el nodo 9
          break;
      case NODO_10:
        if(NODO_FINAL==7)
        {
          if(angulo == 270)
          {
            entrega_palet();
            //reiniciar dijkstra
          }
        }
        break;
      case NODO_11:
        if(NODO_FINAL==8)
        {
          if(angulo == 270)
          {
            entrega_palet();
          }
        }
        break;
      case NODO_12:
        if(NODO_FINAL==9 || NODO_FINAL==16)
        {
          if(angulo == 90 || angulo == 270)
          {
            entrega_palet();
            //reiniciar dijkstra
          }
        }
        break;
      case NODO_13:
          if(NODO_FINAL==14)
          {
            if(angulo == 180)
              {
                recogida_palet();
                //reiniciar dijkstra
              }
          }
          break;
      case NODO_14:
          // Código para el nodo 14
          break;
      case NODO_15:
          if(angulo == 90)
            { 
              vTaskDelay(pdMS_TO_TICKS(200));
              deshabilitar_tareas_para_nodo_especial();
              Serial.println("Preparando WALL-E para pasar por la puerta.");
              Serial.println("Encendemos el IR para abrir la puerta");
              for(int i=0; i<16; i++){ACCION_BABY_ESP(ABRIR_PUERTA_IF); vTaskDelay(pdMS_TO_TICKS(250));}
              baseSpeed = 125; // Para que pase la puerta
              
              vTaskDelay(pdMS_TO_TICKS(1500));
              baseSpeed = 85;
            }
          break;
      case NODO_16:
          // Código para el nodo 16
          break;
      case NODO_17:
          // Código para el nodo 17
          break;
      case NODO_18:
        if(angulo == 0 || angulo == 180 || angulo == 360)
          {
              
            detectar_punto_caliente();
          }
          break;
      case NODO_19:
          // Código para el nodo 19
        if(NODO_FINAL==16)
        {
          if(angulo == 270)
          {
            entrega_palet();
            //reiniciar dijkstra
          }
        }
        break;
      case NODO_20:
        if(angulo == 0 || angulo == 180)
        {
            
          detectar_punto_caliente();
        }
        if(NODO_FINAL==21)
        {
          if(angulo == 180)
            {
              recogida_palet();
              //reiniciar dijkstra
            }
        }
        break;
      case NODO_21:
          break;
      default:
          // Código para manejar otros casos si es necesario
          break;
    }
}

void deshabilitar_tareas_para_nodo_especial()
{
  //Aqui  en el codigo real hay que desactivar el forward tanto para el par de motores A como el B
  forwardA(0);forwardB(0);
  DETECTOR_DE_CRUCE_ENABLE = false;
  SEGUIR_LINEA_ENABLE = false;
  CAMARA_TERMOGRAFICA_ENABLE = false;
  forwardA(0);forwardB(0);
}

void detectar_punto_caliente()
{
  vTaskDelay(pdMS_TO_TICKS(250));
  deshabilitar_tareas_para_nodo_especial();
  Serial.println("Hay que comprobar punto caliente");
  int pos_inicial, pos_final;
  if((node_state < 10 && theta_state == 180) || (node_state > 10 && theta_state == 0))
  {
    pos_inicial = 1800;
    pos_final = 2200;
  }
  else
  {
    pos_inicial = 3600;
    pos_final = 4000;
  }
  Serial.println("Moviendo la camara");
  CAMARA_TERMOGRAFICA_ENABLE = true;
  for(int i = pos_inicial; i<=pos_final; i = i+200)
  {
    st.RegWritePosEx(1, i, 3400, 50);
    st.RegWriteAction();
    vTaskDelay(pdMS_TO_TICKS(1000));
    while(MEDICION_CAMARA_REALIZADA == false){vTaskDelay(pdMS_TO_TICKS(50));}
    MEDICION_CAMARA_REALIZADA = false;
    if(PUNTO_CALIENTE == true)
    {
    Serial.println("¡¡¡QUE TE QUEMAS, CORREEEEEEEE!!!");
    ACCION_BABY_ESP(ACTIVAR_ZUMBADOR);
    break;
    }
    else
    {
      Serial.println("No hay FUEGO BRO");
    }  
  }
  CAMARA_TERMOGRAFICA_ENABLE = false;
}

void recogida_palet()
{
 // Recoger pallet
  Serial.println("RECOGER PALET");
  forwardA(0);forwardB(0);
  DETECTOR_DE_CRUCE_ENABLE = false;
  SEGUIR_LINEA_ENABLE = false;
  forwardA(0);forwardB(0);
  /*st.RegWritePosEx(2, 2700, 3400, 50);//servo(ID2) speed=3400，acc=50，move to position=4095.
  st.RegWriteAction();
  vTaskDelay(pdMS_TO_TICKS(2000));*/
  //El palet estaba arriba --> lo bajamos
  st.RegWritePosEx(2, 3800, 3400, 50);//servo(ID2) speed=3400，acc=50，move to position=4095.
  st.RegWriteAction();
  vTaskDelay(pdMS_TO_TICKS(2000));
  
  SEGUIR_LINEA_ENABLE = true;
  vTaskDelay(pdMS_TO_TICKS(150));
  DETECTOR_DE_CRUCE_ENABLE = true;

  if (xSemaphoreTake(semaforo_cruce, portMAX_DELAY) == pdTRUE)
    {
    DETECTOR_DE_CRUCE_ENABLE = false;
    SEGUIR_LINEA_ENABLE = false;
    forwardA(0);forwardB(0);
    st.RegWritePosEx(2, 3650, 3400, 50);//servo(ID2) speed=3400，acc=50，move to position=4095.
    st.RegWriteAction();
    vTaskDelay(pdMS_TO_TICKS(1000));
    Serial.println("Palet cogido");  
    }
  else
  {}
  
  // Vamos hacia atras
  backwardA(85);backwardB(85);
  vTaskDelay(pdMS_TO_TICKS(300));
  DETECTOR_DE_CRUCE_ENABLE = true;

  if (xSemaphoreTake(semaforo_cruce, portMAX_DELAY) == pdTRUE)
  {
  DETECTOR_DE_CRUCE_ENABLE = false;
  SEGUIR_LINEA_ENABLE = false;
  SENTIDO_MOVIMIENTO = true;
  // Llegamos al nodo anterior
  backwardA(0);backwardB(0);
  vTaskDelay(pdMS_TO_TICKS(200)); 
  }
  forwardA(100);forwardB(100);
  vTaskDelay(pdMS_TO_TICKS(225)); 
  backwardA(0);backwardB(0);
  
  switch(node_state)
  {
    case 5:
      FUNCION_GIRO(-90);
      theta_state=90;
      break;
    case 13:
      FUNCION_GIRO(-90);
      theta_state=90;
      break;
    case 20:
      FUNCION_GIRO(90);
      theta_state=270;
      break;
  }
  STATE_Movement_and_Functions = IDLE_MOVEMENT_AND_FUNCTIONS_STATE;
  xQueueSend(xQueue_STATE_Movement_and_Functions, &STATE_Movement_and_Functions,0);
} 

void entrega_palet()
{ // Entregar pallet
  DETECTOR_DE_CRUCE_ENABLE = false;
  SEGUIR_LINEA_ENABLE = false;
  st.RegWritePosEx(2, 3650, 3400, 50);//servo(ID2) speed=3400，acc=50，move to position=4095.
  st.RegWriteAction();
  vTaskDelay(pdMS_TO_TICKS(1000));
  Serial.println("ENTREGAR PALET");
  
  SEGUIR_LINEA_ENABLE = true;
  SENTIDO_MOVIMIENTO = true;
  vTaskDelay(pdMS_TO_TICKS(200));
  DETECTOR_DE_CRUCE_ENABLE = true;

  if (xSemaphoreTake(semaforo_cruce, portMAX_DELAY) == pdTRUE)
  {
    DETECTOR_DE_CRUCE_ENABLE = false;
    SEGUIR_LINEA_ENABLE = false;
    // Hemos llegado al punto, retrocedemos un poco y dejamos el pallet
    backwardA(100);backwardB(100);
    vTaskDelay(pdMS_TO_TICKS(235));
    backwardA(0);backwardB(0);
    //El palet esta arriba --> lo bajamos
    st.RegWritePosEx(2, 3800, 3400, 50);//servo(ID2) speed=3400，acc=50，move to position=4095.
    st.RegWriteAction();
  }
  backwardA(85);backwardB(85);
  vTaskDelay(pdMS_TO_TICKS(300));
  DETECTOR_DE_CRUCE_ENABLE = true;
  
  if (xSemaphoreTake(semaforo_cruce, portMAX_DELAY) == pdTRUE)
  {
  DETECTOR_DE_CRUCE_ENABLE = false;
  SEGUIR_LINEA_ENABLE = false;
  SENTIDO_MOVIMIENTO = true;
  // Llegamos al nodo anterior
  backwardA(0);backwardB(0);
  //El palet esta abajo --> lo subimos
  st.RegWritePosEx(2, 2700, 3400, 50);//servo(ID2) speed=3400，acc=50，move to position=4095.
  st.RegWriteAction();
  }
  forwardA(100);forwardB(100);
  vTaskDelay(pdMS_TO_TICKS(250)); 
  backwardA(0);backwardB(0);
  STATE_Movement_and_Functions = IDLE_MOVEMENT_AND_FUNCTIONS_STATE;
  xQueueSend(xQueue_STATE_Movement_and_Functions, &STATE_Movement_and_Functions,0);
}


uint8_t pin_I2C_SDA = 32;
uint8_t pin_I2C_SCL = 33;
uint32_t clock_frequency = 100000;
uint16_t ADDRESS_SLAVE = 0x55;
void ACCION_BABY_ESP(uint8_t accion)
{
  Wire.begin(pin_I2C_SDA, pin_I2C_SCL);
  Wire.setClock(clock_frequency);
  Wire.beginTransmission(0x55); // transmit to device #4
  Wire.write(accion);              // sends one byte
  Wire.endTransmission(true);    // stop transmitting
  //Wire.end();
  
}