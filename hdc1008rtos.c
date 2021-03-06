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
<<<<<<< HEAD
 Msg msg;

/* Function Prototypes */
// initialization functions
void message_init();
void i2c_config();
void fatfs_config();
// loop functions
void readSensorData();
void writeSensorData();
=======
uint16_t temperatureBuffer[BUFFER_SIZE] = {0,0,0,0,0,0,0,0,0,0};
uint16_t humidityBuffer[BUFFER_SIZE] = {0,0,0,0,0,0,0,0,0,0};
uint16_t dataCount = 0;



/* SD Card Setup */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
/* Buffer size used for the file copy process */
#ifndef DATA_MSG_SIZE
#define DATA_MSG_SIZE       256
#endif

/* String conversion macro */
#define STR_(n)             #n
#define STR(n)              STR_(n)

/* Drive number used for FatFs */
#define DRIVE_NUM           0

const char  datafile[] = "fat:"STR(DRIVE_NUM)":data.txt";



>>>>>>> I2C

/*
 *  ======== main ========
 */
int main(void)
{
<<<<<<< HEAD
    /* Call board init functions. */
    Board_initGeneral();
    Board_initGPIO();
    Board_initI2C();
    Board_initSDSPI();
    /* Call framework init functions. */
    message_init();
=======
	/*
	 *   ======== writeSensorBuffer ========
	 *
	 */

	/*  Prologue  */
	//SDSPI_Handle sdspiHandle;
	//SDSPI_Params sdspiParams;
	//FILE * dst;
	unsigned int bytesWritten = 0;
	const char formatMsg[] = "(%2i.) Temp Register: Raw = 0x%4x, Celcius = %.3f\n";
	char dataMsg[sizeof(formatMsg)];
	double trouble = 0.0;
	uint8_t i = 0;

	 /* Mount and register the SD Card */
	/*SDSPI_Params_init(&sdspiParams);
	sdspiHandle = SDSPI_open(Board_SDSPI0, DRIVE_NUM, &sdspiParams);
	if (sdspiHandle == NULL) {
		System_abort("Error starting the SD card\n");
	}
	else {
		System_printf("Drive %u is mounted\n", DRIVE_NUM);
	}*/

	/* Create a new file object for the data transfer */
	/*dst = fopen(datafile, "w");
	if (!dst) {
		System_printf("Error opening \"%s\"\n", datafile);
		System_abort("Aborting...\n");
	}
	else {
		System_printf("Starting data transfer to SD Card\n");
	}*/
>>>>>>> I2C

    /* Turn on user LED to indicate successful init  */
    GPIO_write(Board_LED0, Board_LED_ON);

<<<<<<< HEAD
    while(1){
    	readSensorData();
    	writeSensorData();
    }
=======
	/*  Loop      */
	while(true){
		Semaphore_pend(semaWrite, BIOS_WAIT_FOREVER); // this semaphore is the synchronization flag from the read task
		/* Write Process */
		System_printf("Writing Buffer to SD Card\n");
		System_flush();
		for (i=0; i<BUFFER_SIZE; i++) {
			trouble = (temperatureBuffer[i]/65536.0)*165.0 - 40.0;
			sprintf(dataMsg, formatMsg, i+1, temperatureBuffer[i], trouble);
			// System output
			System_printf("(%2i.) Temp Register: Raw = 0x%4x", i+1, temperatureBuffer[i]);
			System_printf(", Celcius = %.3f\n", trouble);
			System_flush();

			// SD Card output
			/*  Write to dst file */
			//bytesWritten = fwrite(dataMsg, 1, sizeof(dataMsg), dst);

		}
>>>>>>> I2C


    return (0);
}

<<<<<<< HEAD
void readSensorData(){
	/* Read Data Process */
	int i = 0;
	for(i=0;i<BUFFER_SIZE;i++){
		msg.humidityBuffer[i] = 0x66CA+i; // humidity = 40%
		msg.temperatureBuffer[i] = 0x60F4+i; // humidity = 40%
		msg.dataCount = i+1;
=======
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
	uint16_t i;
    uint32_t tempRaw;
    double trouble;
	uint16_t config;

	// Create I2C handle for usage
	I2C_Params_init(&i2cParams);
	i2cParams.bitRate = I2C_100kHz;
	i2c = I2C_open(Board_I2C1, &i2cParams);
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
	System_flush();

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
>>>>>>> I2C
	}
	/* End Data Read */
}

void writeSensorData(){
	/* Write Data Process */
	int i = 0;
	double temp, hmd;
	for(i=0;i<BUFFER_SIZE;i++){
		temp = msg.temperatureBuffer[i];
		temp = (temp/65535.0)*165.0-40.0;
		hmd = msg.humidityBuffer[i];
		hmd = (hmd/65535.0)*100;
	}
	/* End Data Write */
}

<<<<<<< HEAD
void message_init(){
/* Primes a single-buffered Queue */
	/* Create and initialize data buffers */
    uint16_t msgTempBufA[BUFFER_SIZE];
    uint16_t msgHmdyBufA[BUFFER_SIZE];
    int i;
    for(i=0; i<BUFFER_SIZE; i++){ msgTempBufA[i] = 1; }
    for(i=0; i<BUFFER_SIZE; i++){ msgHmdyBufA[i] = 2; }
=======
void toggleLedFxn()
{
	while(1){
		GPIO_toggle(Board_LED0);
		Task_sleep(10);
	}
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
>>>>>>> I2C

    /* Compile the messages to store the data buffers */
    msg.temperatureBuffer = msgTempBufA;
    msg.humidityBuffer = msgHmdyBufA;
    msg.dataCount = 0; // increments up to BUFFER_SIZE
}
