/**
Library for reading and writting more than one byte I2C Arduino.
@author Vadim Coselev
*/
  
  #include "i2c_read_registers.h"
  
  SuperI2C::SuperI2C(uint8_t pin_I2C_SDA_INIT, uint8_t pin_I2C_SCL_INIT, uint16_t clock_frequency_INIT, uint8_t SLAVE_ADDRESS_INIT){
    pin_I2C_SDA = pin_I2C_SDA_INIT;
    pin_I2C_SCL = pin_I2C_SCL_INIT;
    clock_frequency = clock_frequency_INIT;
    SLAVE_ADDRESS = SLAVE_ADDRESS_INIT;

    Wire1.begin(pin_I2C_SDA, pin_I2C_SCL);
    Wire1.setClock(clock_frequency);
  };

  uint8_t SuperI2C::Write_Register_I2C(vector<uint8_t> REGISTER_DIRECTION_VECTOR, vector<uint8_t> DATA_TO_WRITE){

    vector<uint8_t> write_vector;
    write_vector = REGISTER_DIRECTION_VECTOR;
    write_vector.insert(write_vector.end(), DATA_TO_WRITE.begin(),DATA_TO_WRITE.end());
    uint8_t* write_array = &write_vector[0];
    Wire1.beginTransmission(SLAVE_ADDRESS);
    Wire1.write(write_array,write_vector.size());
    uint8_t return_int = Wire1.endTransmission(true);
    
    return return_int;

  }


  vector<byte> SuperI2C::Read_Register_I2C(vector<uint8_t> REGISTER_DIRECTION_VECTOR , uint8_t bytes_to_read){

    uint8_t* REGISTER_XY = &REGISTER_DIRECTION_VECTOR[0];
    Wire1.beginTransmission(SLAVE_ADDRESS);

    uint8_t bytes_written = Wire1.write(REGISTER_XY,REGISTER_DIRECTION_VECTOR.size());
    /*Serial.print("Number of bytes added to the end of the transmit buffer: ");
    Serial.println(bytes_written);*/

    if(bytes_written != REGISTER_DIRECTION_VECTOR.size()){
      Serial.println("The number of bytes written are not the same as the data input size. Something went wrong.");
    }

    uint8_t endTransmission_status = Wire1.endTransmission(false);
    /*if(endTransmission_status == 0){
      Serial.println("Success");
    }
    else if(endTransmission_status == 1){
      Serial.println("Data too long to fit in transmit buffer");
    }
    else if(endTransmission_status == 2){
      Serial.println("Received NACK on transmit of address");
    }
    else if(endTransmission_status == 3){
      Serial.println("Received NACK on transmit of data");
    }
    else{Serial.println("Other error");}*/
    uint8_t n_bytes_received = Wire1.requestFrom(static_cast<uint16_t>(SLAVE_ADDRESS),static_cast<size_t>(bytes_to_read)); //SOLAMENTE HABRA UN MAESTRO
    
    if(n_bytes_received != bytes_to_read){
      Serial.println("The number of bytes received and the the ones to read don't match.");
    }
    vector<uint8_t> bytes_received(bytes_to_read);
    byte * buff = &bytes_received[0];
    Wire1.readBytes(buff, bytes_to_read);
    /*while(Wire1.available()){
    for(uint8_t i = 0; i<= bytes_received.size()-1; i++){
      buff[i] = Wire1.read();
    }
    }*/
    vector<uint8_t> buff_vector(buff, buff + sizeof buff / sizeof buff[0]); 

    return bytes_received;

  }