/**
Principal State Machine triggered by Wi-Fi inputs.
@author Vadim Coselev
@author Asier Legaz
*/

#include "global_definitions.h"
#include "Maquina_De_Estados_Movimiento.h"
#include "tareas.h"
#include "ruta_mas_corta.h"

#include <WiFi.h>
#include <WebServer.h>
#include "WebPage.h"
WebServer server(80);
#include "ArduinoJson.h"

StaticJsonDocument<256> jsonCmdReceive;
StaticJsonDocument<256> jsonInfoSend;
TaskHandle_t serialCtrlHandle;

const char* AP_SSID = "WALL-E";
const char* AP_PWD  = "12345678";

uint8_t NODO_FINAL = 0;

//Creamos estructura para poder pasar de forma facil el vector de dijkstra

//Task Handles for suspending and executing each task
TaskHandle_t State_Machine_For_WiFi_ISR_Handle = NULL;

TaskHandle_t State_Machine_For_Movement_and_Functions_Handle = NULL;


int16_t MAT_GIRO[22][22]= {
/*0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21*/ 
{0,270,0,0,0,0,0,0,0,0,180,0,0,0,0,0,0,0,0,0,0,0,},/*1*/
{90,0,180,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},/*2*/
{0,0,0,180,0,0,0,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},/*3*/
{0,0,0,0,180,0,0,0,90,0,0,0,0,0,0,0,0,0,0,0,0,0,},/*4*/
{0,0,0,0,0,180,0,0,0,90,0,0,0,0,0,0,0,0,0,0,0,0,},/*5*/
{0,0,0,0,0,0,180,0,0,0,0,0,0,90,0,0,0,0,0,0,0,0,},/*6*/
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},/*7*/
{0,0,270,0,0,0,0,0,0,0,90,0,0,0,0,0,0,0,0,0,0,0,},/*8*/
{0,0,0,270,0,0,0,0,0,0,0,90,0,0,0,0,0,0,0,0,0,0,},/*9*/
{0,0,0,0,270,0,0,0,0,0,0,0,90,0,0,0,0,0,0,0,0,0,},/*10*/
{0,0,0,0,0,0,0,270,0,0,0,180,0,0,0,0,0,0,0,0,0,0,},/*11*/
{0,0,0,0,0,0,0,0,270,0,0,0,180,0,0,90,0,0,0,0,0,0,},/*12*/
{0,0,0,0,0,0,0,0,0,270,0,0,0,180,0,0,90,0,0,0,0,0,},/*13*/
{0,0,0,0,0,270,0,0,0,0,0,0,0,0,180,0,0,0,0,0,90,0,},/*14*/
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},/*15*/
{0,0,0,0,0,0,0,0,0,0,0,270,0,0,0,0,0,0,90,0,0,0,},/*16*/
{0,0,0,0,0,0,0,0,0,0,0,0,270,0,0,0,0,0,0,90,0,0,},/*17*/
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,180,0,0,0,},/*18*/
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,270,0,0,0,180,0,0,},/*19*/
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,270,0,0,0,180,0,},/*20*/
{0,0,0,0,0,0,0,0,0,0,0,0,0,270,0,0,0,0,0,0,0,180,},/*21*/
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},/*22*/
};
/*
int16_t MAT_ADYACENTE[22][22]= {
{0,58,0,0,0,0,0,0,0,0,45,0,0,0,0,0,0,0,0,0,0,0,},
{58,0,45,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
{0,45,0,56,0,0,0,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
{0,0,56,0,46,0,0,0,29,0,0,0,0,0,0,0,0,0,0,0,0,0,},
{0,0,0,46,0,28,0,0,0,29,0,0,0,0,0,0,0,0,0,0,0,0,},
{0,0,0,0,28,0,26,0,0,0,0,0,0,58,0,0,0,0,0,0,0,0,},
{0,0,0,0,0,26,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
{0,0,29,0,0,0,0,0,0,0,29,0,0,0,0,0,0,0,0,0,0,0,},
{0,0,0,29,0,0,0,0,0,0,0,29,0,0,0,0,0,0,0,0,0,0,},
{0,0,0,0,29,0,0,0,0,0,0,0,29,0,0,0,0,0,0,0,0,0,},
{45,0,0,0,0,0,0,29,0,0,0,56,0,0,0,0,0,0,0,0,0,0,},
{0,0,0,0,0,0,0,0,29,0,56,0,46,0,0,29,0,0,0,0,0,0,},
{0,0,0,0,0,0,0,0,0,29,0,46,0,28,0,0,28,0,0,0,0,0,},
{0,0,0,0,0,58,0,0,0,0,0,0,28,0,26,0,0,0,0,0,59,0,},
{0,0,0,0,0,0,0,0,0,0,0,0,0,26,0,0,0,0,0,0,0,0,},
{0,0,0,0,0,0,0,0,0,0,0,29,0,0,0,0,0,0,29,0,0,0,},
{0,0,0,0,0,0,0,0,0,0,0,0,28,0,0,0,0,0,0,29,0,0,},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,56,0,0,0,},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,29,0,56,0,46,0,0,},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,29,0,46,0,28,0,},
{0,0,0,0,0,0,0,0,0,0,0,0,0,59,0,0,0,0,0,28,0,26,},
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,0,},
};
*/

int16_t MAT_ADYACENTE[22][22]= {
// 0   1   2   3   4   5   6   7   8   9   10   11   12   13   14   15   16   17   18   19   20   21   
  {0,  58, 0,  0,  0,  0,  0,  0,  0,  0,  45,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,}, //0
  {58, 0,  45, 0,  0,  0,  0,  0,  0,  0,   0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,}, //1
  {0,  45, 0,  56, 0,  0,  0,  99, 0,  0,   0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,}, //2
  {0,  0,  56, 0,  46, 0,  0,  0,  99, 0,   0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,}, //3
  {0,  0,  0,  46, 0,  28, 0,  0,  0,  99,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,}, //4
  {0,  0,  0,  0,  28, 0,  26, 0,  0,  0,   0,  0,   0,   58,  0,   0,   0,   0,   0,   0,   0,   0,}, //5
  {0,  0,  0,  0,  0,  26, 0,  0,  0,  0,   0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,}, //6
  {0,  0,  99, 0,  0,  0,  0,  0,  0,  0,   99, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,}, //7
  {0,  0,  0,  99, 0,  0,  0,  0,  0,  0,   0,  99,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,}, //8
  {0,  0,  0,  0,  99, 0,  0,  0,  0,  0,   0,  0,   99,  0,   0,   0,   0,   0,   0,   0,   0,   0,}, //9
  {45, 0,  0,  0,  0,  0,  0,  99, 0,  0,   0,  56,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,}, //10
  {0,  0,  0,  0,  0,  0,  0,  0,  99, 0,   56, 0,   46,  0,   0,   40,  0,   0,   0,   0,   0,   0,}, //11
  {0,  0,  0,  0,  0,  0,  0,  0,  0,  99,  0,  46,  0,   28,  0,   0,   99,  0,   0,   0,   0,   0,}, //12
  {0,  0,  0,  0,  0,  58, 0,  0,  0,  0,   0,  0,   28,  0,   26,  0,   0,   0,   0,   0,   59,  0,}, //13
  {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,   0,   26,  0,   0,   0,   0,   0,   0,   0,   0,}, //14
  {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  40,  0,   0,   0,   0,   0,   0,   40,  0,   0,   0,}, //15
  {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,   99,  0,   0,   0,   0,   0,   0,   99,  0,   0,}, //16
  {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,   0,   0,   0,   0,   0,   0,   56,  0,   0,   0,}, //17
  {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,   0,   0,   0,   40,  0,   56,  0,   0,   0,   0,}, //18
  {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,   0,   0,   0,   0,   99,  0,   0,   0,   28,  0,}, //19
  {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,   0,   59,  0,   0,   0,   0,   0,   28,  0,   26,},//20
  {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   26,  0,}, //21
};


uint8_t STATE_Main_WiFi_ISR = 0;
QueueHandle_t xQueue_STATE_Main_WiFi_ISR = xQueueCreate(1, sizeof(&STATE_Main_WiFi_ISR));

void setup() {
  Serial.begin(115200);


  WiFi.disconnect();
  delay(100);

  WiFi.softAP(AP_SSID, AP_PWD);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  xTaskCreate(&commandThreading, "serialCtrl", 4000, NULL, 5, &serialCtrlHandle);

  webCtrlServer();




  BaseType_t State_Machine_For_WiFi_ISR = xTaskCreate(
                                                vTaskState_Machine_For_WiFi_ISR,              //function name
                                                "Main_Machine_WiFi_Interruptions",            //task name
                                                4096,                                          //stack size
                                                nullptr,                                      //task parameters
                                                10,                                            //task priority
                                                &State_Machine_For_WiFi_ISR_Handle            //task handle
                                              );

  BaseType_t Simulador_Cruce = xTaskCreate(
                                                vSimulador_Cruce,              //function name
                                                "Simulador_Cruce",            //task name
                                                1024,                                          //stack size
                                                nullptr,                                      //task parameters
                                                8,                                            //task priority
                                                nullptr            //task handle
                                              );
  BaseType_t Seguir_Linea = xTaskCreate(
                                                vSeguir_Linea,              //function name
                                                "Seguir_Linea",            //task name
                                                8000,                                          //stack size
                                                nullptr,                                      //task parameters
                                                8,                                            //task priority
                                                nullptr            //task handle
                                              );

  BaseType_t State_Machine_For_Movement_and_Functions = xTaskCreate(
                                            vTaskState_Machine_For_Movement_and_Functions,              //function name
                                            "State_Machine_For_Movement_and_Functions",            //task name
                                            2048,                                          //stack size
                                            nullptr,                                      //task parameters
                                            9,                                            //task priority
                                            &State_Machine_For_Movement_and_Functions_Handle                                       //task handle
                                          );

  BaseType_t xMLX90641_Read = xTaskCreate(
                                          MLX90641_Read, 
                                          "MLX90641_Read", 
                                          4095, 
                                          nullptr, 
                                          10, 
                                          nullptr);

  /*BaseType_t Laser_Read = xTaskCreate(
                                        vLaser_Read,            //function name
                                        "Laser_Read",         //task name
                                        4096,                 //stack size
                                        nullptr,             //task parameters
                                        10,                 //task priority
                                        nullptr            //task handle
                                      );*/

  TaskHandle_t DeteccionCruce_Handle;
  
  //INICIALIZACION MOTORES
    initMotors();

    // Calibramos sensores
    vTaskDelay(pdMS_TO_TICKS(500));
    Serial.begin(115200);
    Serial.println("Comienza calibracion en 2 segundos");
    vTaskDelay(pdMS_TO_TICKS(2000));
    calibracion();
    Serial.println("Calibrado. Comienza en 5 segundos.");
    vTaskDelay(pdMS_TO_TICKS(5000));

    //uint8_t INICIALIZACION_DEBUG = VUELTA_DE_RECONOCIMIENTO;
    //xQueueSend(xQueue_STATE_Main_WiFi_ISR, (void *)&INICIALIZACION_DEBUG, 0);

    

}

void loop() {
  //detectarCruce();
  //seguirLinea();
  vTaskDelay(pdMS_TO_TICKS(10));

}



////////////////////////////////////////////////////
/*
COMIENZO MAQUINA DE ESTADO MAIN_WiFi_ISR
*/
///////////////////////////////////////////////////
//uint8_t STATE_Main_WiFi_ISR = 0;
std::vector<int> vector_recogida_entrega(8);
std::vector<int16_t> vector_auxiliar_dijkstra;
struct AVector estructura_dijkstra_vector;

void vTaskState_Machine_For_WiFi_ISR(void * pvParameters)
{
    /*
    1 -> IDLE_WIFI_MAIN
    2 -> VUELTA DE RECONOCIMIENTO
    3 -> RECOGER Y ENTREGAR PALLETS
    4 -> APARCAR
    */
    //vector_recogida_entrega.at(6) = 15;
    vector_recogida_entrega.at(6) = 17;
    while (true)
    {
      BaseType_t  STATE_Main_WiFi_ISR_Trigger;
      STATE_Main_WiFi_ISR_Trigger = xQueueReceive(xQueue_STATE_Main_WiFi_ISR, &STATE_Main_WiFi_ISR, 0);
      if(STATE_Main_WiFi_ISR_Trigger == pdPASS)
      {
        switch (STATE_Main_WiFi_ISR)
        {
        case IDLE_WIFI_MAIN:
            Serial.println("IDLE_WIFI_MAIN");
            vector_recogida_entrega.erase(vector_recogida_entrega.begin());
            if(vector_recogida_entrega.empty()==0)
            {
              STATE_Main_WiFi_ISR = RECOGER_ENTREGAR_PALLETS;
              xQueueSend(xQueue_STATE_Main_WiFi_ISR, (void *)&STATE_Main_WiFi_ISR, 0);
            }
            break;
        case VUELTA_DE_RECONOCIMIENTO: //VUELTA DE RECONOCIMIENTO
            node_state = 0;
            theta_state = 180;
            WIFI_NODOS_ESPECIALES_SELECCION = 1;
            Serial.println("VUELTA DE RECONOCIMIENTO");
            
            estructura_dijkstra_vector.vector_dijkstra = {0, 0, 10, 0, 11, -90, 15, 0, 18, 90, 19, 0, 20, 90, 13, 0, 5, 90, 4, 0, 3, 0, 2};
            //estructura_dijkstra_vector.vector_dijkstra = {2, 0, 3, 0,4,0};
            //estructura_dijkstra_vector.vector_dijkstra = {1, 0, 2, 0, 3, 0, 4, 0, 5, -90, 13, 0, 20, -90};
            xQueueReset(Queue_Handle_Vector_Dijkstra);
            xQueueSend(Queue_Handle_Vector_Dijkstra, (void *)&estructura_dijkstra_vector,(TickType_t)0);
            STATE_Movement_and_Functions = RECEIVE_ARRAY_STATE;
            xQueueSend(xQueue_STATE_Movement_and_Functions, &STATE_Movement_and_Functions,0);

            break;
        case RECOGER_ENTREGAR_PALLETS:
            WIFI_NODOS_ESPECIALES_SELECCION = 0;
            Serial.println("RECOGER Y ENTREGAR PALLETS");
            NODO_FINAL = vector_recogida_entrega[0];
            vector_auxiliar_dijkstra = dijkstra_vector_function(node_state, theta_state, NODO_FINAL, MAT_ADYACENTE, MAT_GIRO);
            estructura_dijkstra_vector.vector_dijkstra.assign(vector_auxiliar_dijkstra.begin(),vector_auxiliar_dijkstra.end());
            xQueueReset(Queue_Handle_Vector_Dijkstra);
            xQueueSend(Queue_Handle_Vector_Dijkstra, (void *)&estructura_dijkstra_vector,(TickType_t)0);
            STATE_Movement_and_Functions = RECEIVE_ARRAY_STATE;
            xQueueSend(xQueue_STATE_Movement_and_Functions, &STATE_Movement_and_Functions,0);
            break;
        case PUERTA_Y_APARCAR:
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
////////////////////////////////////////////////////
/*
FIN MAQUINA DE ESTADO MAIN_WiFi_ISR
*/
///////////////////////////////////////////////////


void webCtrlServer()
{
  server.on("/",  handleRoot);
  server.on("/cmd", handleCMD);
  server.begin();
  Serial.println("Server Starts.");
}

void handleRoot()
{
  server.send(200, "text/html", index_html); //Send web page
  Serial.println("connecting...");
}

void handleCMD()
{
    if (server.args() > 0) {
        for (uint8_t i = 0; i < server.args(); i++) {
            String argName = server.argName(i);
            String argValue = server.arg(i);

            // Procesar cada parámetro de la solicitud
            if (argName == "Accion") {
              // Procesar el valor de la acción
              uint8_t cmdAValue = argValue.toInt();
              xQueueSend(xQueue_STATE_Main_WiFi_ISR, (void *)&cmdAValue, 0);
            } 
            else if (argName == "Nodo_actual") {
              // Procesar el valor del nodo actual
              node_state = argValue.toInt();
              Serial.print("Nodo Actual: "); Serial.println(node_state);
            } 
            else if (argName == "Orientacion_actual") {
              // Procesar el valor de la orientación actual
              theta_state = argValue.toInt();
              Serial.print("Orientación actual: "); Serial.println(theta_state);
            }
            else if (argName == "Entrega_azul") {
              // Procesar el valor del punto de entrega del palet azul
              vector_recogida_entrega.at(0) = 6;
              vector_recogida_entrega.at(1) = argValue.toInt();
            }
            else if (argName == "Entrega_rojo") {
              // Procesar el valor del punto de entrega del palet azul
              vector_recogida_entrega.at(2) = 14;
              vector_recogida_entrega.at(3) = argValue.toInt();
            }
            else if (argName == "Entrega_verde") {
              //Procesar el valor del punto de entrega del palet azul
              vector_recogida_entrega.at(4) = 21;
              vector_recogida_entrega.at(5) = argValue.toInt();
            }
        }
    }

    server.send(200, "text/plain", "");
}

void commandThreading(void *pvParameter){
  while(1)
  {
    server.handleClient();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}


