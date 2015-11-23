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

/* Constants */
#define BUFFER_SIZE 10
#define Board_HDC1008_ADDR 0x40

typedef struct hdcMsg {
	Queue_Elem elem;
	int * temperatureBuffer;
	int * humidityBuffer;
	int bufferLength;
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
	 *   Use the I2C driver to communicate with
	 */

/*  Prologue  */
	I2C_Handle      i2c;
	I2C_Params      i2cParams;
	I2C_Transaction i2cTransaction;
	uint8_t txBuffer[1] = {0};   // stores the pointer to the register to read from
	uint8_t rxBuffer[2] = {0,0}; // stores one 16-bit integer
	Msg * hdcMsg; //queue message

	// Create I2C for usage
	I2C_Params_init(&i2cParams);
	i2cParams.bitRate = I2C_400kHz;
	i2c = I2C_open(Board_I2C0, &i2cParams);
	if (i2c == NULL) {
		System_abort("Error Initializing I2C\n");
	}
	else {
		System_printf("I2C Initialized!\n");
	}
	// For the base I2C module, make sure that SDA = P1.6 and SCL = P1.7 are connected
    i2cTransaction.slaveAddress = Board_HDC1008_ADDR; // A0=A1=0 on the device for 0x40 address
	i2cTransaction.writeBuf = txBuffer;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readBuf = rxBuffer;
	i2cTransaction.readCount = 2;

	// Initialize the configuration register of the HDC1008
	txBuffer[0] = 0x02; //configuration register address
	if (I2C_transfer(i2c, &i2cTransaction)) {
		System_printf("Config Register: MSB = 0x%x , LSB = 0x%x", rxBuffer[0], rxBuffer[1]);
	}
	else {
		System_printf("I2C Bus fault\n");
	}
	System_flush();

	hdcMsg = Queue_get(toReadQueue);

/*  Loop      */
	while(true){
		Semaphore_pend(semaRead, BIOS_WAIT_FOREVER); // this semaphore is dependent on the clock module's tick
		/* Sensor Read Process */
		hdcMsg = Queue_get(toReadQueue);

		//if(bufferLength>=10){

		//	Semaphore_post(semaWrite);
		//}
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
