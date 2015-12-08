/*
 * hdc1008_config.h
 *
 *  Created on: Nov 24, 2015
 *      Author: Eric
 */

/****************************************************
  This is a library for the HDC1000 Humidity & Temp Sensor

  Designed specifically to work with the HDC1000 sensor from Adafruit
  ----> https://www.adafruit.com/products/2635

  These sensors use I2C to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#ifndef HDC1008_CONFIG_H_
#define HDC1008_CONFIG_H_

// derived from Adafruit's library for Arduino
#define HDC1008_I2CADDR 0x40
#define HDC1008_TEMP          0x00
#define HDC1008_HUMID         0x01
#define HDC1008_CONFIG        0x02
#define HDC1008_CONFIG_RST    (1 << 15)
#define HDC1008_CONFIG_HEAT   (1 << 13)
#define HDC1008_CONFIG_MODE   (1 << 12)
#define HDC1008_CONFIG_BATT   (1 << 11)
#define HDC1008_CONFIG_TRES_14  0
#define HDC1008_CONFIG_TRES_11  (1 << 10)
#define HDC1008_CONFIG_HRES_14  0
#define HDC1008_CONFIG_HRES_11  (1 << 8)
#define HDC1008_CONFIG_HRES_8   (1 << 9)

#endif /* HDC1008_CONFIG_H_ */
