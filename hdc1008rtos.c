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

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/SDSPI.h>


/* Board Header file */
#include "Board.h"

/* Other Libraries */
#include "hdc1008_config.h"

/* Constants */
#define BUFFER_SIZE 10


typedef struct hdcMsg {
	Queue_Elem elem;
	uint16_t * temperatureBuffer;
	uint16_t * humidityBuffer;
	uint16_t bufferLength;
} Msg;



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
	Msg hdcMsgW = *((Msg *)Queue_get(toReadQueue));
	uint16_t i;
	uint16_t temp;
	uint16_t hmd;
	for (i=0; i<BUFFER_SIZE; i++){
		temp = (hdcMsgW.temperatureBuffer[i]/65535.0)*165-40;
		hmd = (hdcMsgW.humidityBuffer[i]/65535.0)*100;
		System_printf("%i.) Raw temp = 0x%x, real temp = %i degC\t",i,hdcMsgW.temperatureBuffer[i],temp);
		System_printf("|| Raw hmdy = 0x%x, real hmdy = %i percent\n",i,hdcMsgW.humidityBuffer[i],hmd);
		System_flush();
	}
	System_printf("=======\nwriteSensorBuffer Task is Ready...\n=======\n");
	System_flush();

	/*  Loop      */
	while(true){
		Semaphore_pend(semaWrite, BIOS_WAIT_FOREVER); // this semaphore is the synchronization flag from the read task
		hdcMsgW.bufferLength = 0; // reset the buffer length for the read task to use
		Queue_put(toReadQueue, &(hdcMsgW.elem)); // send out a next copy of the message
		hdcMsgW = *((Msg *)Queue_get(toWriteQueue)); // get the data-rich message
		System_printf("\nwriteTask: getting a new message at 0x%x\n", &hdcMsgW);
		/* Write Process */
		for (i=0; i<BUFFER_SIZE; i++){
			temp = (hdcMsgW.temperatureBuffer[i]/65535.0)*165-40;
			hmd = (hdcMsgW.humidityBuffer[i]/65535.0)*100;
			System_printf("%i.) Raw temp = 0x%x, real temp = %i degC\t",i,hdcMsgW.temperatureBuffer[i],temp);
			System_printf("|| Raw hmdy = 0x%x, real hmdy = %i percent\n",i,hdcMsgW.humidityBuffer[i],hmd);
			System_flush();
		}


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
	 *   Use the I2C driver to communicate with
	 */

/*  Prologue  */
	Msg hdcMsgR; //queue message
	hdcMsgR = *((Msg *)Queue_get(toReadQueue));
	uint16_t i;
	uint8_t msgSent = 0;
	for (i=0; i<BUFFER_SIZE; i++){
		temp = (hdcMsgR.temperatureBuffer[i]/65535.0)*165-40;
		hmd = (hdcMsgR.humidityBuffer[i]/65535.0)*100;
		System_printf("%i.) Raw temp = 0x%x, real temp = %i degC\t",i,hdcMsgR.temperatureBuffer[i],temp);
		System_printf("|| Raw hmdy = 0x%x, real hmdy = %i percent\n",i,hdcMsgR.humidityBuffer[i],hmd);
		System_flush();
	}
	
	System_printf("=======\nreadSensorBuffer Task is Ready...\n=======\n");
	System_flush();


/*  Loop      */
	while(true){
		Semaphore_pend(semaRead, BIOS_WAIT_FOREVER); // this semaphore is dependent on the clock module's tick
		if(msgSent){
			hdcMsgR = *((Msg *)Queue_get(toReadQueue));	// get a new message
			msgSent = 0;
			System_printf("\nreadTask: getting a new message at 0x%x\n", &hdcMsgR);
			System_flush();
		}


		hdcMsgR.temperatureBuffer[hdcMsgR.bufferLength] = 0x6043;
		hdcMsgR.humidityBuffer[hdcMsgR.bufferLength] = 0x8000;

		System_flush();
		Task_sleep(100);
		hdcMsgR.bufferLength += 1;

		if(hdcMsgR.bufferLength >= BUFFER_SIZE){
			//hdcMsg = Queue_get(toReadQueue);
			Queue_put(toWriteQueue, &(hdcMsgR.elem));
			msgSent = 1; // turn on flag that the message is empty
			Semaphore_post(semaWrite);
		}


	}

/*  Epilogue  */

}

/*
 * ============== Read Sensor Clock Swi ====================
 */
void readTimerFxn()
{
	// let the sensor read task know that it
	//System_printf("\nTimer HWI activated\n");
	//System_flush();
	Semaphore_post(semaRead);
}

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
    uint16_t msgTempBufA[BUFFER_SIZE];
    uint16_t msgTempBufB[BUFFER_SIZE];
    uint16_t msgHmdyBufA[BUFFER_SIZE];
    uint16_t msgHmdyBufB[BUFFER_SIZE];
    int i;
    for(i=0; i<BUFFER_SIZE; i++){ msgTempBufA[i] = 1; }
    for(i=0; i<BUFFER_SIZE; i++){ msgTempBufB[i] = 2; }
    for(i=0; i<BUFFER_SIZE; i++){ msgHmdyBufA[i] = 3; }
    for(i=0; i<BUFFER_SIZE; i++){ msgHmdyBufB[i] = 4; }

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


    BIOS_start();

    return (0);
}
