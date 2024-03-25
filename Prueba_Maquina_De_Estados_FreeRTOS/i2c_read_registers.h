/**
Library for reading and writting more than one byte I2C Arduino.
@author Vadim Coselev
*/

#ifndef I2C_READ_REGISTERS_DEFINITION
#define I2C_READ_REGISTERS_DEFINITION

#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>
#include <vector>
#include "global_definitions.h"

using namespace std;

class SuperI2C
{
  private:
    uint8_t pin_I2C_SDA;
    uint8_t pin_I2C_SCL;
    uint16_t clock_frequency;
    uint8_t SLAVE_ADDRESS;
  
  public:
    SuperI2C(uint8_t pin_I2C_SDA_INIT = 38, uint8_t pin_I2C_SCL_INIT = 39, uint16_t clock_frequency_INIT = 100000, uint8_t SLAVE_ADDRESS_INIT = 0x33);
    uint8_t Write_Register_I2C(vector<uint8_t> REGISTER_DIRECTION_VECTOR, vector<uint8_t> DATA_TO_WRITE);
    vector<byte> Read_Register_I2C(vector<uint8_t> REGISTER_DIRECTION_VECTOR, uint8_t bytes_to_read);
};
#endif