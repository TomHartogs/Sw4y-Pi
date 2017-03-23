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

struct CAM {
	image_format_t m_fmt;
	sensor_model_t sensor_model;
	uint8_t sensor_addr;
};

static struct CAM myCAM;

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
			//arducam_write_reg(ARDUCHIP_TEST1, 0x55, CAM1_CS);
				//arducam_spi_write(ARDUCHIP_TEST1 | 0x80, 0x55, CAM1_CS);
			uint8_t spiData[2];
			spiData[0] = ARDUCHIP_TEST1 | 0x80;
			spiData[1] = 0x55;
			if (SPI_CS < 0)
				wiringPiSPIDataRW(SPI_ARDUCAM, spiData, 2);
			else
			{

				digitalWrite(CAM1_CS, LOW);
				wiringPiSPIDataRW(SPI_ARDUCAM, spiData, 2);
				digitalWrite(CAM1_CS, HIGH);
			}

			uint8_t temp;
			//temp = arducam_read_reg(ARDUCHIP_TEST1, CAM1_CS);
				//data = arducam_spi_read(addr & 0x7F, SPI_CS);
			uint8_t spiData[2];
			spiData[0] = ARDUCHIP_TEST1 & 0x7F;
			spiData[1] = 0x00;
			if (SPI_CS < 0)
				wiringPiSPIDataRW(SPI_ARDUCAM, spiData, 2);
			else
			{
				digitalWrite(CAM1_CS, LOW);
				wiringPiSPIDataRW(SPI_ARDUCAM, spiData, 2);
				digitalWrite(CAM1_CS, HIGH);
			}
			temp =  spiData[1];

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
