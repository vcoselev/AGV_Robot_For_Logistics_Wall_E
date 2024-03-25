/**
State Machine for Movements.
@author Vadim Coselev
@author Asier Legaz
*/
#ifndef MAQUINA_DE_ESTADOS_MOVIMIENTO_DEFINICION
#define MAQUINA_DE_ESTADOS_MOVIMIENTO_DEFINICION

#include "global_definitions.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>


extern uint8_t STATE_Movement_and_Functions;
extern QueueHandle_t xQueue_STATE_Movement_and_Functions;

//Definimos la maquina de estados con sus respectivos estados
void vTaskState_Machine_For_Movement_and_Functions(void * pvParameters);
#define RECEIVE_ARRAY_STATE 1
#define IDLE_MOVEMENT_AND_FUNCTIONS_STATE 2
#define ARRAY_SHIFT_STATE 3
#define GIRO_STATE 4
#define FORWARD_STATE 5
#define TEST_SPECIAL_FUNCTION_STATE 6


//Definimos el numero de nodos que vamos a usar
#define NODO_0 0
#define NODO_1 1
#define NODO_2 2
#define NODO_3 3
#define NODO_4 4
#define NODO_5 5
#define NODO_6 6
#define NODO_7 7
#define NODO_8 8
#define NODO_9 9
#define NODO_10 10
#define NODO_11 11
#define NODO_12 12
#define NODO_13 13
#define NODO_14 14
#define NODO_15 15
#define NODO_16 16
#define NODO_17 17
#define NODO_18 18
#define NODO_19 19
#define NODO_20 20
#define NODO_21 21



extern TaskHandle_t IDLE_MOVEMENT_AND_FUNCTIONS_Handle;
extern bool IDLE_MOVEMENT_AND_FUNCTIONS_TASK_ENABLE;
void vIDLE_MOVEMENT_AND_FUNCTIONS(void * pvParameters);

void nodos_especial_vuelta_reconocimiento(uint8_t nodo, int16_t angulo);
void nodos_especial_recogida_entrega(uint8_t nodo, int16_t angulo);
void deshabilitar_tareas_para_nodo_especial();

void detectar_punto_caliente();
void recogida_palet();
void entrega_palet();
void ACCION_BABY_ESP(uint8_t accion);




#endif