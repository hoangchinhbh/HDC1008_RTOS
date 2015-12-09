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
	txBuffer[0] = HDC1008_TEMP; // write the temperature register's address to the HDC1008's pointer register for SoC
	if(!write(1)){
		return 0xFFF0; // error writing to point register
	}
	int timeout = 0; // watchdog timer
	// wait for the data to be ready via polling
	while(!GPIO_read(dataReadyPin)){
		if(timeout>=0xFFFFFFFFF){
			//return 0xFFF1; //timeout error
		}
		timeout++;
	}
	if(!read(2)){
		return 0xFFF2; // error reading from temperature register
	}
	return rxBuffer[0]<<8 | rxBuffer[1];
}

uint16_t HDC1008::getRawHumidity(){
	txBuffer[0] = HDC1008_HUMID; // write the humidity register's address to the HDC1008's pointer register for SoC
	if(!write(1)){
		return 0xAAA0; // error writing to point register
	}
	int timeout = 0; // watchdog timer
	// wait for the data to be ready via polling
	while(!GPIO_read(dataReadyPin)){
		if(timeout>=0xFFFFFFFFF){
			//return 0xAAA1; //timeout error
		}
		timeout++;
	}
	if(!read(2)){
		return 0xAAA2; // error reading from humidity register
	}
	return rxBuffer[0]<<8 | rxBuffer[1];
}

double HDC1008::getRealTemperatureC(uint16_t raw){
	return (raw/65535)*165.0-40.0; // transfer function from datasheet
}

double HDC1008::getRealTemperatureF(uint16_t raw){
	return ((raw/65535)*165.0-40.0)*1.8 + 32.0; // transfer function for deg C --> F
}

double HDC1008::getRealHumidity(uint16_t raw){
	return (raw/65535)*100.0; //transfer function from datasheet
}

/*
 *    Private Functions
 */
uint8_t HDC1008::defaultConfig(){
	uint8_t flag;
	config = HDC1008_CONFIG_DEFAULT;
	txBuffer[0] = HDC1008_CONFIG;
	txBuffer[1] = config >> 8;
	txBuffer[2] = config & 0x00FF;
	flag = write(3);
	return flag;
}

uint8_t HDC1008::write(uint8_t length){
	i2cTransaction.writeCount = length;
	i2cTransaction.readCount = 0;
	return I2C_transfer(i2c, &i2cTransaction);
}

uint8_t HDC1008::read(uint8_t length){
	i2cTransaction.writeCount = 0;
	i2cTransaction.readCount = length;
	return I2C_transfer(i2c, &i2cTransaction);
}
