/**
FreeRTOS tasks for reading sensors.
Calibration
@author Vadim Coselev
@author Asier Legaz
*/
#ifndef SENSORES_DEFINITION
#define SENSORES_DEFINITION
#include "global_definitions.h"
#include <freertos/semphr.h>


extern SemaphoreHandle_t semaforo_cruce;
void enviar_IR_puerta();


void forwardA(uint16_t pwm);
void backwardA(uint16_t pwm);
void forwardB(uint16_t pwm);
void backwardB(uint16_t pwm);
void initMotors();
void calibracion(void);
void FUNCION_GIRO(int16_t angulo);



void vSimulador_Cruce(void * pvParameters);
extern bool DETECTOR_DE_CRUCE_ENABLE;

void vSeguir_Linea(void * pvParameters);
extern bool SEGUIR_LINEA_ENABLE;
extern bool SENTIDO_MOVIMIENTO;
extern uint16_t baseSpeed;

void MLX90641_Read(void * pvParameters);
extern bool CAMARA_TERMOGRAFICA_ENABLE;
extern bool PUNTO_CALIENTE;
extern bool MEDICION_CAMARA_REALIZADA;

//void vLaser_Read(void * pvParameters);
//extern bool LASER_ENABLE;


#endif