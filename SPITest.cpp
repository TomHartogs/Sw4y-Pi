#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <wiringPiSPI.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <unistd.h>

typedef enum {
	fmtBMP,
	fmtJPEG
} image_format_t;

typedef enum {
	smOV2640,
} sensor_model_t;

typedef enum {
	sz160x120,
	sz176x144,
	sz320x240,
	sz352x288,
	sz640x480,
	sz800x600,
	sz1024x768,
	sz1280x960,
	sz1600x1200,
} jpeg_size_t;

struct CAM {
	image_format_t m_fmt;
	sensor_model_t sensor_model;
	uint8_t sensor_addr;
};

//****************************************************/
//* Camera Choose Pin Definition
//****************************************************/
//   CAM_CS  |    BCM   |   wPi
/****************************************************/
//   CAM1_CS |    5     |    21
//---------------------------------------------------/
//   CAM2_CS |    6     |    22
//---------------------------------------------------/
//   CAM3_CS |    12    |    26
//---------------------------------------------------/
//   CAM4_CS |    13    |    23
/****************************************************/

#define CAM1_CS 21

#define SPI_ARDUCAM 1
#define SPI_ARDUCAM_SPEED 1000000

#define ARDUCHIP_TEST1       	0x00  //TEST register
#define ARDUCHIP_TEST2      	0x01  //TEST register

static struct CAM myCAM;

void arducam_spi_write(uint8_t address, uint8_t value, int SPI_CS);

void arducam_write_reg(uint8_t addr, uint8_t data, int SPI_CS)
{
	arducam_spi_write(addr | 0x80, data, SPI_CS);
}

void arducam_spi_write(uint8_t address, uint8_t value, int SPI_CS)
{
	uint8_t spiData[2];
	spiData[0] = address;
	spiData[1] = value;
	if (SPI_CS < 0)
		wiringPiSPIDataRW(SPI_ARDUCAM, spiData, 2);
	else
	{

		digitalWrite(SPI_CS, LOW);
		wiringPiSPIDataRW(SPI_ARDUCAM, spiData, 2);
		digitalWrite(SPI_CS, HIGH);
	}
}

uint8_t arducam_spi_read(uint8_t address, int SPI_CS);

uint8_t arducam_read_reg(uint8_t addr, int SPI_CS)
{
	uint8_t data;
	data = arducam_spi_read(addr | 0x80, SPI_CS);

	return data;
}

uint8_t arducam_spi_read(uint8_t address, int SPI_CS)
{
	uint8_t spiData[2];
	spiData[0] = address;
	spiData[1] = 0x00;
	if (SPI_CS < 0)
		wiringPiSPIDataRW(SPI_ARDUCAM, spiData, 2);
	else
	{
		digitalWrite(SPI_CS, LOW);
		wiringPiSPIDataRW(SPI_ARDUCAM, spiData, 2);
		digitalWrite(SPI_CS, HIGH);
	}
	return spiData[1];
}

int main()
{
	wiringPiSetup();
	if (wiringPiSPISetup(SPI_ARDUCAM, SPI_ARDUCAM_SPEED) != -1)
	{
		myCAM.sensor_model = smOV2640;
		myCAM.sensor_addr = 0x30;
		pinMode(CAM1_CS, OUTPUT);
		digitalWrite(CAM1_CS, HIGH);

		if (wiringPiI2CSetup(myCAM.sensor_addr) != -1)
		{
			arducam_write_reg(ARDUCHIP_TEST1, 0x55, CAM1_CS);
			uint8_t temp;
			temp = arducam_read_reg(ARDUCHIP_TEST1, CAM1_CS);
			printf("temp=%x\n",temp);
			if (temp != 0x55) {
				printf("SPI interface error!\n");
				exit(EXIT_FAILURE);
			}
			else {
				printf("SPI interface OK!\n");
			}
		}
		else
		{
			printf("ERROR: I2C init failed\n");
		}
	}
	else
	{
		printf("ERROR: SPI init failed\n");
	}
	return 0;
}
