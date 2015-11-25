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
#include "math.h"

/* Constants */
#define BUFFER_SIZE 10
#define Board_HDC1008_ADDR 0x40

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
	uint8_t txBuffer[3] = {0,0,0};   // [0] stores the pointer to the register to read from
	uint8_t rxBuffer[4] = {0,0,0,0}; // stores one 16-bit integer
	Msg * hdcMsg; //queue message

	// Create I2C for usage
	I2C_Params_init(&i2cParams);
	i2cParams.bitRate = I2C_100kHz;
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

	// Read the default configuration register setting
	txBuffer[0] = 0x02; //configuration register address
	if (I2C_transfer(i2c, &i2cTransaction)) {
		System_printf("Config Register: MSB = 0x%x , LSB = 0x%x\n", rxBuffer[0], rxBuffer[1]);
	}
	else {
		System_printf("I2C Bus fault\n");
	}
	Task_sleep(10); // wait for the device to come online
	System_printf("readSensorBuffer Task is Ready...\n");
	System_flush();

	hdcMsg = Queue_get(toReadQueue);
    int i;
    int tempRaw;
/*  Loop      */
	while(true){
		//Semaphore_pend(semaRead, BIOS_WAIT_FOREVER); // this semaphore is dependent on the clock module's tick
		/* Sensor Read Process */
		//hdcMsg = Queue_get(toReadQueue);
		//if(bufferLength>=10){

		//	Semaphore_post(semaWrite);
		//}
		// Read the default configuration register setting

		txBuffer[0] = 0x00; // temperature register
		i2cTransaction.readCount = 0;
		if (I2C_transfer(i2c, &i2cTransaction)) {
			Task_sleep(100);
			System_printf("Successfully wrote 0x00 to the pointer\n");
		} else { System_printf("I2C Bus fault - Writing to temp pointer\n"); }
		i2cTransaction.readCount = 2;
		i2cTransaction.writeCount = 0;

		for(i=0; i<BUFFER_SIZE; i++){
			if (I2C_transfer(i2c, &i2cTransaction)) {
				hdcMsg->temperatureBuffer[i] = rxBuffer[0] + (rxBuffer[1]<<8);
				tempRaw = hdcMsg->temperatureBuffer[i];
				System_printf("%i.)   Temp Register: Raw = 0x%x", i+1, hdcMsg->temperatureBuffer[i]);
				//Task_setPri(readSensorBuffer, 10);
				hdcMsg->temperatureBuffer[i] = (tempRaw/pow(2.0,16.0))*165.0 - 40.0;
				//Task_setPri(readSensorBuffer, 2);
				System_printf(", Celcius = %i, %x\n", hdcMsg->temperatureBuffer[i], hdcMsg->temperatureBuffer[i]);
			} else { System_printf("I2C Bus fault - Reading from temp\n====\n"); }

			System_flush();
			Task_sleep(100);
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
    temp = (temp/pow(2.0,16.0))*165.0 - 40.0;
    System_printf("The temperature for 0x6043 is %i degC\n", temp);
    System_flush();
    /* Start BIOS */
    BIOS_start();

    return (0);
}
