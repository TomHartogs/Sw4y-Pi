/*-----------------------------------------

//Update History:
//2016/06/13 	V1.1	by Lee	add support for burst mode

--------------------------------------*/
#include "arducam.h"
#include "arducam_arch.h"
#include "memorysaver.h"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
 

static struct CAM myCAM; 

int arducam(sensor_model_t model,int SPI_CS1, int SPI_CS2, int SPI_CS3, int SPI_CS4)
{
	myCAM.sensor_model = model;
	switch(myCAM.sensor_model)
	{
		case smOV7660:
		case smOV7670:
		case smOV7675:
		case smOV7725:
			myCAM.sensor_addr = 0x21;
			break;
		case smMT9D111:
			myCAM.sensor_addr = 0x5d;
			break;
		case smOV3640:
		case smOV5642:
			myCAM.sensor_addr = 0x3c;
			break;
		case smOV2640:
		case smOV9655:
			myCAM.sensor_addr = 0x30;
			break;
		case smMT9M112:
			myCAM.sensor_addr = 0x5d;
			break;
		default:
			myCAM.sensor_addr = 0x21;
			break;
	}
	if(SPI_CS1>=0)
		{
			if (!arducam_spi_init(SPI_CS1)) {
				printf("ERROR: SPI1 init failed\n");
				//return 0;
			}
	 }
	 if(SPI_CS2>=0)
	 	{
			if (!arducam_spi_init(SPI_CS2)) {
				printf("ERROR: SPI2 init failed\n");
				//return 0;
			}
   }
   if(SPI_CS3>=0)
   	{
			if (!arducam_spi_init(SPI_CS3)) {
				printf("ERROR: SPI3 init failed\n");
				//return 0;
			}
    }
    if(SPI_CS4>=0)
     {
				if (!arducam_spi_init(SPI_CS4)) {
					printf("ERROR: SPI4 init failed\n");
					//return 0;
				}
     }

	// initialize i2c:
	if (!arducam_i2c_init(myCAM.sensor_addr)) {
		printf("ERROR: I2C init failed\n");
		//return 0;
	}
	return 1;
}

void arducam_init()
{
	arducam_i2c_write(0xff, 0x01);
	arducam_i2c_write(0x12, 0x80);
	arducam_delay_ms(100);
	if(myCAM.m_fmt == fmtJPEG)
	{
		arducam_i2c_write_regs(OV2640_JPEG_INIT);
		arducam_i2c_write_regs(OV2640_YUV422);
		arducam_i2c_write_regs(OV2640_JPEG);
		arducam_i2c_write(0xff, 0x01);
		arducam_i2c_write(0x15, 0x00);
		arducam_i2c_write_regs(OV2640_320x240_JPEG);
	}
	else
	{
		arducam_i2c_write_regs(OV2640_QVGA);
	}
}

void arducam_flush_fifo(int SPI_CS)
{
	arducam_write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK, SPI_CS);
}

void arducam_start_capture(int SPI_CS)
{
	arducam_write_reg(ARDUCHIP_FIFO, FIFO_START_MASK, SPI_CS);
}

void arducam_clear_fifo_flag(int SPI_CS)
{
	arducam_write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK,SPI_CS);
}

uint32_t read_fifo_length(int SPI_CS)
{
	
	uint32_t len1,len2,len3,length=0;
	len1 = arducam_read_reg(FIFO_SIZE1, SPI_CS);
  len2 = arducam_read_reg(FIFO_SIZE2, SPI_CS);
  len3 = arducam_read_reg(FIFO_SIZE3, SPI_CS) & 0x07;
  length = ((len3 << 16) | (len2 << 8) | len1) & 0x07ffff;
	return length;
	
}

void arducam_transfer(uint8_t data)
{
  arducam_spi_transfer(data);
}

void arducam_transfers(uint8_t *buf, uint32_t size)
{
	arducam_spi_transfers(buf, size);
}

void set_fifo_burst(uint8_t data)
{
	arducam_transfer(data);
}



uint8_t arducam_read_fifo(int SPI_CS)
{
	uint8_t data;
	data = arducam_spi_read(0x3D, SPI_CS);
	return data;
}

uint8_t arducam_read_reg(uint8_t addr,int SPI_CS)
{
	uint8_t data;
	data = arducam_spi_read(addr & 0x7F, SPI_CS);
	
	return data;
}

void arducam_write_reg(uint8_t addr, uint8_t data, int SPI_CS)
{
	arducam_spi_write(addr | 0x80, data, SPI_CS);
}

//My add
//Set corresponding bit  
void set_bit(uint8_t addr, uint8_t bit, int SPI_CS)
{
	uint8_t temp;
	temp = arducam_read_reg(addr, SPI_CS);
	arducam_write_reg(addr, temp | bit, SPI_CS);

}
//Clear corresponding bit 
void clear_bit(uint8_t addr, uint8_t bit, int SPI_CS)
{
	uint8_t temp;
	temp = arducam_read_reg(addr,SPI_CS);
	arducam_write_reg(addr, temp & (~bit), SPI_CS);

}

void arducam_set_jpeg_size(jpeg_size_t size)
{
	switch(size)
	{
		case sz160x120:
			arducam_i2c_write_regs(OV2640_160x120_JPEG);
			break;
		case sz176x144:
			arducam_i2c_write_regs(OV2640_176x144_JPEG);
			break;
		case sz320x240:
			arducam_i2c_write_regs(OV2640_320x240_JPEG);
			break;
		case sz352x288:
			arducam_i2c_write_regs(OV2640_352x288_JPEG);
			break;
		case sz640x480:
			arducam_i2c_write_regs(OV2640_640x480_JPEG);
			break;
		case sz800x600:
			arducam_i2c_write_regs(OV2640_800x600_JPEG);
			break;
		case sz1024x768:
			arducam_i2c_write_regs(OV2640_1024x768_JPEG);
			break;
		case sz1280x960:
			arducam_i2c_write_regs(OV2640_1280x960_JPEG);
			break;
		case sz1600x1200:
			arducam_i2c_write_regs(OV2640_1600x1200_JPEG);
			break;
		default:
			arducam_i2c_write_regs(OV2640_320x240_JPEG);
			break;
	}
}

void arducam_set_format(image_format_t fmt)
{
	myCAM.m_fmt = fmt;
}