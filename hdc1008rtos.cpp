/*
 *    HDC1008 Logger
 *    Written by: Eric Bauer, Gus Fragasse, Joe Warner
 */

/*
 *  ======== hdc1008rtos.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/SDSPI.h>

/* Board Header file */
#include "Board.h"

/* Other Libraries */
#include "libs/hdc1008_config.h"
#include "libs/myQueue.h"
#include "libs/HDC1008.h"

/* Constants */
#define BUFFER_SIZE 10
#define DATA_RDY 2

/* Global Variables */
uint16_t temperatureBuffer[BUFFER_SIZE] = {0,0,0,0,0,0,0,0,0,0};
uint16_t humidityBuffer[BUFFER_SIZE] = {0,0,0,0,0,0,0,0,0,0};
uint16_t dataCount = 0;
HDC1008 hdcSensor(DATA_RDY);

/* Function Prototypes */
// initialization functions
void i2c_config();
void fatfs_config();
// loop functions
void readSensorData();
void writeSensorData();

/*
 *  ======== main ========
 */
int main(void)
{
    /* Call board init functions. */
    Board_initGeneral();
    Board_initGPIO();
    Board_initI2C();
    Board_initSDSPI();
    /* Framework init */


    /* Turn on user LED to indicate successful init  */
    GPIO_write(Board_LED0, Board_LED_ON);

    while(1){
    	readSensorData();
    	dataCount++;
    	if(dataCount>=BUFFER_SIZE){
    		dataCount = 0;
    		writeSensorData();
    	}

    }


    return (0);
}

void readSensorData(){
	/* Read Data Process */
	humidityBuffer[dataCount] = 0xABCD; //hdcSensor.getRawTemperature();
	temperatureBuffer[dataCount] = 0x1337; //hdcSensor.getRawHumidity();
	/* End Data Read */
}

void writeSensorData(){
	/* Write Data Process */
	int i = 0;
	double temp= 0.0;
	double hmd = 0.0;
	for(i=0;i<BUFFER_SIZE;i++){
		temp = temperatureBuffer[i];
		temp = (temp/65535.0)*165.0-40.0;
		hmd = humidityBuffer[i];
		hmd = (hmd/65535.0)*100;
	}
	/* End Data Write */
}

