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

/* Global Variables */
uint16_t temperatureBuffer[BUFFER_SIZE] = {0,0,0,0,0,0,0,0,0,0};
uint16_t humidityBuffer[BUFFER_SIZE] = {0,0,0,0,0,0,0,0,0,0};
uint16_t dataCount = 0;

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

	System_printf("=======\nwriteSensorBuffer Task is Ready...\n=======\n");
	System_flush();

	/*  Loop      */
	while(true){
		Semaphore_pend(semaWrite, BIOS_WAIT_FOREVER); // this semaphore is the synchronization flag from the read task
		/* Write Process */
		System_printf(":D Look at me, I'm writing data!\n");
		System_flush();
		double trouble = 0.0;
		uint8_t i = 0;
		for (i=0; i<BUFFER_SIZE; i++) {
			System_printf("(%2i.) Temp Register: Raw = 0x%4x", i+1, temperatureBuffer[i]);
			trouble = (temperatureBuffer[i]/65536.0)*165.0 - 40.0;
			System_printf(", Celcius = %.3f\n", trouble);
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
	I2C_Handle      i2c;
	I2C_Params      i2cParams;
	I2C_Transaction i2cTransaction;
	Msg * hdcMsg; //queue message
	uint8_t txBuffer[3] = {0,0,0};   // [0] stores the pointer to the register to read from
	uint8_t rxBuffer[4] = {0,0,0,0}; // stores one 16-bit integer
	uint16_t i;
    uint32_t tempRaw;
    double trouble;
	uint16_t config;

	// Create I2C handle for usage
	I2C_Params_init(&i2cParams);
	i2cParams.bitRate = I2C_100kHz;
	i2c = I2C_open(Board_I2C0, &i2cParams);
	if (i2c == NULL) { System_abort("Error Initializing I2C\n");
	} else { System_printf("I2C Initialized!\n"); }
	
	// For the base I2C module, make sure that SDA = P1.6 and SCL = P1.7 are connected
    i2cTransaction.slaveAddress = HDC1008_I2CADDR; // A0=A1=0 on the device for 0x40 address
	i2cTransaction.writeBuf = txBuffer;	
	i2cTransaction.readBuf = rxBuffer;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readCount = 2;

	// Read the default configuration register setting
	txBuffer[0] = 0x02; //configuration register address
	if (I2C_transfer(i2c, &i2cTransaction)) {
		System_printf("Initial Config Register: MSB = 0x%x , LSB = 0x%x\n", rxBuffer[0], rxBuffer[1]);
		config = rxBuffer[1]+(rxBuffer[0]<<8);
	} else { System_printf("I2C Bus fault -- reading config register (initially)\n"); }
	
	// Write configuration register settings for Single,  14-bit measurement
	txBuffer[0] = 0x02; //configuration register address
	config = 0x0000; //HDC1008_CONFIG_RST | HDC1008_CONFIG_TRES_14 | HDC1008_CONFIG_HRES_14;
	txBuffer[1] = config>>8; // MSB of data to write
	txBuffer[2] = config & 0x00FF; // LSB of data to write
	i2cTransaction.writeCount = 3;
	if (I2C_transfer(i2c, &i2cTransaction)) {
		System_printf("Adjusted Config Register: MSB = 0x%x , LSB = 0x%x\n", rxBuffer[0], rxBuffer[1]);
		config = rxBuffer[1]+(rxBuffer[0]<<8);
	} else { System_printf("I2C Bus fault -- reading config register (adjusted)\n"); }
	
	System_printf("=======\nreadSensorBuffer Task is Ready...\n=======\n");
	System_flush();

	hdcMsg = Queue_get(toReadQueue);

/*  Loop      */
	while(true){
		Semaphore_pend(semaRead, BIOS_WAIT_FOREVER); // this semaphore is dependent on the clock module's tick

		for (i=0; i<BUFFER_SIZE; i++) {
			tempRaw = 0;

			// Initiate a temperature reading
			txBuffer[0] = 0x00;
			i2cTransaction.readCount = 0;
			i2cTransaction.writeCount = 1;
			if (I2C_transfer(i2c, &i2cTransaction)) {
				//System_printf("Succesfully wrote 0x00 to the pointer!\n");
			} else {
				System_printf("I2C Bus fault - Writing to temp pointer\n");
			}

			// Wait 10 ms
			Task_sleep(10);

			// Perform a temperature read
			txBuffer[0] = 0;
			i2cTransaction.readCount = 2;
			i2cTransaction.writeCount = 0;

			if (I2C_transfer(i2c, &i2cTransaction)) {
				// [0] is MSB, [1] is LSB
				tempRaw = (rxBuffer[0] << 8) | rxBuffer[1];
				System_printf("(%2i.) [0]: 0x%2x ; [1] 0x%2x", i+1, rxBuffer[0], rxBuffer[1]);
				// Grab top 14 bits
				System_printf(", Temp Register: Raw = 0x%4x", tempRaw);
				//Task_setPri(readSensorBuffer, 10);
				// tempRaw = ((tempRaw * 165) / (65536)) - 40;
				trouble = (tempRaw/65536.0)*165.0 - 40.0;
				//Task_setPri(readSensorBuffer, 2);
				System_printf(", Celcius = %.3f\n", trouble);

				// filling the buffer
				temperatureBuffer[i] = tempRaw;
			} else {
				System_printf("I2C Bus fault - Reading from temp pointer\n");
			}

			System_flush();
		}

		Semaphore_post(semaWrite);
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

    uint16_t temp = 0x43+(0x60<<8);
    System_printf("The result of MSB = 0x60 and LSB = 0x43 is: %x\n", temp);
    temp = (temp/65536.0)*165.0 - 40.0;
    System_printf("The temperature for 0x6043 is %i degC\n", temp);
    System_flush();
    /* Start BIOS */
    BIOS_start();

    return (0);
}
