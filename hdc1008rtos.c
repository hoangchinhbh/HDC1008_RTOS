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

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/SDSPI.h>


/* Board Header file */
#include "Board.h"

/* Constants */
#define BUFFER_SIZE 10

/*
 * ============== Write to SD Card Task ====================
 */

void writeSensorBufferFxn()
{
	/*
	 *   ======== writeSensorBuffer ========
	 *
	 */

	/*  Prologue  */

	/*  Loop      */
	while(true){
		Semaphore_pend(semaWrite, BIOS_WAIT_FOREVER); // this semaphore is the synchronization flag from the read task
		/* Write Process */


	}

	/*  Epilogue  */
}

/*
 * ============== Read HDC1008 Sensor Task ====================
 */
void readSensorBufferFxn()
{
	/*
	 *   ======== readSensorBuffer ========
	 *
	 */

	/*  Prologue  */

	/*  Loop      */
	while(true){
		Semaphore_pend(semaRead, BIOS_WAIT_FOREVER); // this semaphore is dependent on the clock module's tick
		/* Sensor Read Process */


	}

	/*  Epilogue  */
}

/*
 * ============== Read Sensor Clock Swi ====================
 */
void readTimerFxn()
{
	// let the sensor read task know that it
	Semaphore_post(semaRead);
}

typedef struct hdcMsg {
	Queue_Elem elem;
	int * temperatureBuffer;
	int * humidityBuffer;
	int bufferLength;
} Msg;

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

    /* Turn on user LED  */
    GPIO_write(Board_LED0, Board_LED_ON);

    /* Prime a Double-buffered Queue */
    int msgTempBufA[BUFFER_SIZE];
    int msgTempBufB[BUFFER_SIZE];
    int msgHmdyBufA[BUFFER_SIZE];
    int msgHmdyBufB[BUFFER_SIZE];

    Msg msgA;
    msgA.temperatureBuffer = msgTempBufA;
    msgA.humidityBuffer = msgHmdyBufA;
    msgA.bufferLength = 0; // increments up to BUFFER_SIZE
    Msg msgB;
    msgB.temperatureBuffer = msgTempBufB;
    msgB.humidityBuffer = msgHmdyBufB;
	msgB.bufferLength = 0; // increments up to BUFFER_SIZE

    Queue_put(toReadQueue, &(msgA.elem));
    Queue_put(toReadQueue, &(msgB.elem));

    /* Start BIOS */
    BIOS_start();

    return (0);
}
