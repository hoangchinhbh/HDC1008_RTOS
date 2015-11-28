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
	Msg * hdcMsgW = Queue_get(toReadQueue);
	uint16_t i;
	uint16_t temp;
	uint16_t hmd;
	System_printf("=======\nwriteSensorBuffer Task is Ready...\n=======\n");
	System_flush();

	/*  Loop      */
	while(true){
		Semaphore_pend(semaWrite, BIOS_WAIT_FOREVER); // this semaphore is the synchronization flag from the read task
		hdcMsgW->bufferLength = 0; // reset the buffer length for the read task to use
		Queue_put(toReadQueue, &(hdcMsgW->elem)); // send out a next copy of the message
		hdcMsgW = Queue_get(toWriteQueue); // get the data-rich message
		/* Write Process */
		for (i=0; i<hdcMsgW->bufferLength; i++){
			temp = (hdcMsgW->temperatureBuffer[i]/65535.0)*165-40;
			hmd = (hdcMsgW->humidityBuffer[i]/65535.0)*100;
			System_printf("%i.) Raw temp = 0x%x, real temp = %i degC\t",i,hdcMsgW->temperatureBuffer[i],temp);
			System_printf("|| Raw hmdy = 0x%x, real hmdy = %i percent\n",i,hdcMsgW->humidityBuffer[i],hmd);
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
	Msg * hdcMsgR; //queue message
	hdcMsgR = Queue_get(toReadQueue);
	
	System_printf("=======\nreadSensorBuffer Task is Ready...\n=======\n");
	System_flush();


/*  Loop      */
	while(true){
		Semaphore_pend(semaRead, BIOS_WAIT_FOREVER); // this semaphore is dependent on the clock module's tick



		hdcMsgR->temperatureBuffer[hdcMsgR->bufferLength] = 0x6043;
		hdcMsgR->humidityBuffer[hdcMsgR->bufferLength] = 0x8000;

		System_flush();
		Task_sleep(100);
		hdcMsgR->bufferLength += 1;

		if(hdcMsgR->bufferLength >= BUFFER_SIZE){
			//hdcMsg = Queue_get(toReadQueue);
			Queue_put(toWriteQueue, &(hdcMsgR->elem));
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
	System_printf("\nTimer HWI activated\n");
	System_flush();
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
    for(i=0; i<BUFFER_SIZE; i++){ msgTempBufA[i] = 0; }
    for(i=0; i<BUFFER_SIZE; i++){ msgTempBufB[i] = 0; }
    for(i=0; i<BUFFER_SIZE; i++){ msgHmdyBufA[i] = 0; }
    for(i=0; i<BUFFER_SIZE; i++){ msgHmdyBufB[i] = 0; }

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


    uint16_t temp = 0x43+(0x60<<8);
    System_printf("The result of MSB = 0x60 and LSB = 0x43 is: %x\n", temp);
    temp = (temp/65536.0)*165.0 - 40.0;
    System_printf("The temperature for 0x6043 is %i degC\n", temp);
    System_flush();
    /* Start BIOS */
    BIOS_start();

    return (0);
}
