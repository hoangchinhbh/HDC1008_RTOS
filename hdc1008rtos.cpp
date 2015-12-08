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

/* Constants */
#define BUFFER_SIZE 10

/* Global Variables */
 Msg msg;

/* Function Prototypes */
// initialization functions
void message_init();
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
    /* Call framework init functions. */
    message_init();

    /* Turn on user LED to indicate successful init  */
    GPIO_write(Board_LED0, Board_LED_ON);

    while(1){
    	readSensorData();
    	writeSensorData();
    }


    return (0);
}

void readSensorData(){
	/* Read Data Process */
	int i = 0;
	for(i=0;i<BUFFER_SIZE;i++){
		msg.humidityBuffer[i] = 0x1337; //0x66CA; // humidity = 40%
		msg.temperatureBuffer[i] = 0xABCD; //0x60F4; // temperature = 72F
		msg.dataCount = i+1;
	}
	/* End Data Read */
}

void writeSensorData(){
	/* Write Data Process */
	int i = 0;
	double temp= 0.0;
	double hmd = 0.0;
	for(i=0;i<BUFFER_SIZE;i++){
		temp = msg.temperatureBuffer[i];
		temp = (temp/65535.0)*165.0-40.0;
		hmd = msg.humidityBuffer[i];
		hmd = (hmd/65535.0)*100;
	}
	/* End Data Write */
}

void message_init(){
/* Primes a single-buffered Queue */
	/* Create and initialize data buffers */
    uint16_t msgTempBufA[BUFFER_SIZE];
    uint16_t msgHmdyBufA[BUFFER_SIZE];
    int i;
    for(i=0; i<BUFFER_SIZE; i++){ msgTempBufA[i] = 1; }
    for(i=0; i<BUFFER_SIZE; i++){ msgHmdyBufA[i] = 2; }

    /* Compile the messages to store the data buffers */
    msg.temperatureBuffer = msgTempBufA;
    msg.humidityBuffer = msgHmdyBufA;
    msg.dataCount = 0; // increments up to BUFFER_SIZE
}
