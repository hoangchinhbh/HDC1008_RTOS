/*
 *    HDC1008 Logger
 *    Written by: Eric Bauer
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
#include "hdc1008_config.h"
#include "myQueue.h"

/* Constants */
#define BUFFER_SIZE 10

/* Global Variables */
Queue * myQueue;

/* Function Prototypes */
void myQueue_init();
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
    myQueue_init();
    /* Turn on user LED to indicate successful init  */
    GPIO_write(Board_LED0, Board_LED_ON);

    while(1){
    	readSensorData();
    	writeSensorData();
    }


    return (0);
}

void readSensorData(){
	Msg msgR = queueGet(myQueue); // get the message to read data into
	/* Read Data Process */

	/* End Data Read */
	queuePut(myQueue, msgR); // post the message back to the queue
}

void writeSensorData(){
	Msg msgW = queueGet(myQueue); // get the message to write data from
	/* Write Data Process */

	/* End Data Write */
	queuePut(myQueue, msgW); // post the message back to the queue
}

void myQueue_init(){
/* Primes a single-buffered Queue */
	myQueue = createQueue();

	/* Create and initialize data buffers */
    uint16_t msgTempBufA[BUFFER_SIZE];
    uint16_t msgHmdyBufA[BUFFER_SIZE];
    int i;
    for(i=0; i<BUFFER_SIZE; i++){ msgTempBufA[i] = 1; }
    for(i=0; i<BUFFER_SIZE; i++){ msgHmdyBufA[i] = 2; }

    /* Compile the messages to put in the Queue */
    Msg msgA;
    msgA.temperatureBuffer = msgTempBufA;
    msgA.humidityBuffer = msgHmdyBufA;
    msgA.dataCount = 0; // increments up to BUFFER_SIZE

	/* Insert the messages into the Queue */
	queuePut(myQueue, msgA);
}
