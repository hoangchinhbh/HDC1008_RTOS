/*
 * HDC1008.cpp
 *
 *  Created on: Dec 8, 2015
 *      Author: Eric Bauer
 */

#include <libs/HDC1008.h>


HDC1008::HDC1008(uint8_t i_dataReadyPin) {
	// configure the GPIO in order to read when the data is ready
	dataReadyPin = i_dataReadyPin;
	// set dataReadyPin to be input???

	// initialize the rx/tx buffers
	int i;
	for(i=0; i<3; i++){ txBuffer[i] = 0; }
	for(i=0; i<4; i++){ rxBuffer[i] = 0; }

	// Create I2C handle for usage
	I2C_Params_init(&i2cParams);
	i2cParams.bitRate = I2C_100kHz;
	i2c = I2C_open(Board_I2C0, &i2cParams);
	if (i2c == NULL) {
		// print failure to init
	} else {
		// print success to init
	}
	// For the base I2C module, make sure that SDA = P1.6 and SCL = P1.7 are connected
	i2cTransaction.slaveAddress = HDC1008_I2CADDR; // A0=A1=0 on the device for 0x40 address
	i2cTransaction.writeBuf = txBuffer;
	i2cTransaction.readBuf = rxBuffer;

	if(!defaultConfig()){
		// print failure to configure the I2C device properly
	}
}

HDC1008::~HDC1008() {
	// TODO Auto-generated destructor stub
}

/*
 *    Public Functions
 */

uint16_t HDC1008::getRawTemperature(){

}

uint16_t HDC1008::getRawHumidity(){

}

double HDC1008::getRealTemperature(uint16_t raw){

}

double HDC1008::getRealHumidity(uint16_t raw){

}

/*
 *    Private Functions
 */
uint8_t defaultConfig(){
	uint8_t flag;
	config = 0x0000;
	txBuffer[0] = HDC1008_CONFIG;
	txBuffer[1] = config >> 8;
	txBuffer[2] = config & 0x00FF;
	flag = write(3);
	return flag;
}

uint8_t write(uint8_t length){
	i2cTransaction.writeCount = length;
	i2cTransaction.readCount = 0;
	return i2c_transfer(i2c, &i2cTransaction);
}

uint8_t read(uint8_t length){
	i2cTransaction.writeCount = 0;
	i2cTransaction.readCount = length;
	return i2c_transfer(i2c, &i2cTransaction);
}
