/**
Global variables
@author Vadim Coselev
@author Asier Legaz
*/

#include "global_definitions.h"

struct AVector xVector;
extern uint8_t STATE_Main_WiFi_ISR;
extern QueueHandle_t xQueue_STATE_Main_WiFi_ISR;

QueueHandle_t Queue_Handle_Vector_Dijkstra = xQueueCreate(1, sizeof(xVector));
QueueHandle_t xQueue_STATE_Movement_and_Functions = xQueueCreate(1, sizeof(uint8_t));
int16_t theta_state = 180;
uint16_t node_state = 3;
SemaphoreHandle_t Semaphore_I2C = xSemaphoreCreateBinary();