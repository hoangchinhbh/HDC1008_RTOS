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


void readSensorBufferFxn()
{
	/*
	 *   ======== readSensorBuffer ========
	 *
	 */

	/*  Prologue  */

	/*  Loop      */

	/*  Epilogue  */
}

void writeSensorBufferFxn()
{
	/*
	 *   ======== writeSensorBuffer ========
	 *
	 */

	/*  Prologue  */

	/*  Loop      */

	/*  Epilogue  */
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
    int msgTempBufA[10] = {0,0,0,0,0,0,0,0,0,0};
    int msgTempBufB[10] = {0,0,0,0,0,0,0,0,0,0};
    int msgHmdyBufA[10] = {0,0,0,0,0,0,0,0,0,0};
    int msgHmdyBufB[10] = {0,0,0,0,0,0,0,0,0,0};

    Msg msgA;
    msgA.temperatureBuffer = msgTempBufA;
    msgA.humidityBuffer = msgHmdyBufA;
    msgA.bufferLength = 10;
    Msg msgB;
    msgB.temperatureBuffer = msgTempBufB;
    msgB.humidityBuffer = msgHmdyBufB;
	msgB.bufferLength = 10;

    Queue_put(toReadQueue, &(msgA.elem));
    Queue_put(toReadQueue, &(msgB.elem));

    /* Start BIOS */
    BIOS_start();

    return (0);
}
