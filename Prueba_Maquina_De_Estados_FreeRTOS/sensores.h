/**
FreeRTOS tasks for reading sensors and PID controller.
@author Vadim Coselev
@author Asier Legaz
*/

#ifndef SENSORES_DEFINITION
#define SENSORES_DEFINITION
#include "global_definitions.h"
#include <freertos/semphr.h>

extern bool SEGUIR_LINEA_ENABLE;
extern bool DETECTOR_DE_CRUCE_ENABLE;
extern bool COMUNICACION_ENABLE;
extern int ACCION_DSP;
extern SemaphoreHandle_t semaforo_cruce;
void vSimulador_Cruce(void * pvParameters);
void vSeguir_Linea(void * pvParameters);
void forwardA(uint16_t pwm);
void backwardA(uint16_t pwm);
void forwardB(uint16_t pwm);
void backwardB(uint16_t pwm);
void initMotors();
void calibracion(void);
void FUNCION_GIRO(int16_t angulo);

#endif