/**
Global variables definition.
@author Vadim Coselev
@author Asier Legaz
*/
#ifndef GLOBAL_DEFINITIONS_DEFINE
#define GLOBAL_DEFINITIONS_DEFINE
#include <stdint.h>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "QTRSensors.h"
//Librerias propias
#include "Maquina_De_Estados_Movimiento.h"
#include "tareas.h"

//Libreria para el IR de la puerta
#include "src/IRremote/IRremoteESP8266.h"
#include "src/IRremote/IRsend.h"
#include "src/IRremote/ir_NEC.h"

//Libreria Driver I2C
#include "i2c_read_registers.h"


/*
PARA PODER USAR VARIABLES DESDE CUALQUIER ARCHIVO .C (COMO GLOBALES)
HAY QUE DEFINIRLAS EN EL HEADER COMO EXTERN Y VOLVER A DEFINIRLAS EN UNO
DE LOS C O CPP. ESTO NO SE PORQUE ES, PERO ES COSA DE C. SOLO SE DEBE
DEFINIR EN UNO DE LOS ARCHIVOS C, YA QUE SI NO SE VAN A CREAR DIFERENTES
COPIAS DE LA MISMA VARIABLE.

*/

struct AVector
{
  std::vector<int16_t> vector_dijkstra;
};
extern struct AVector xVector;
extern int16_t theta_state; 
extern uint16_t node_state; 
extern QueueHandle_t Queue_Handle_Vector_Dijkstra;
extern QueueHandle_t xQueue_STATE_Movement_and_Functions;
extern SemaphoreHandle_t Semaphore_I2C;

//Definimos los estados de la maquina de estado principal
#define IDLE_WIFI_MAIN 1
#define VUELTA_DE_RECONOCIMIENTO 2
#define RECOGER_ENTREGAR_PALLETS 3
#define PUERTA_Y_APARCAR 4


extern uint8_t WIFI_NODOS_ESPECIALES_SELECCION;
extern uint8_t NODO_FINAL;
extern uint8_t STATE_Main_WiFi_ISR;
extern QueueHandle_t xQueue_STATE_Main_WiFi_ISR;
#endif


