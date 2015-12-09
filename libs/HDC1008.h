/*
 * HDC1008.h
 *
 *  Created on: Dec 8, 2015
 *      Author: Eric
 */
#include <ti/drivers/I2C.h>
#include <ti/drivers/GPIO.h>
#include <stdint.h>
#include <Board.h>
#include "hdc1008_config.h"

#ifndef LIBS_HDC1008_H_
#define LIBS_HDC1008_H_

class HDC1008 {
public:
	HDC1008(uint8_t dataReadyPin);
	virtual ~HDC1008();
	/* User Interfacing Functions */
	uint16_t getRawTemperature();
	uint16_t getRawHumidity();
	double getRealTemperatureC(uint16_t raw);
	double getRealTemperatureF(uint16_t raw);
	double getRealHumidity(uint16_t raw);
private:
	/* Methods */
	uint8_t defaultConfig();
	uint8_t write(uint8_t length);
	uint8_t read(uint8_t length);
	/* Members */
	I2C_Handle      i2c;
	I2C_Params      i2cParams;
	I2C_Transaction i2cTransaction;
	uint8_t txBuffer[3];
	uint8_t rxBuffer[4];
	uint16_t config;
	uint8_t dataReadyPin;
};

#endif /* LIBS_HDC1008_H_ */
