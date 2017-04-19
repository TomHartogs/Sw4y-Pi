/*-----------------------------------------
//Update History:
//2016/06/13 	V1.1	by Lee	add support for burst mode
--------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <wiringPiSPI.h>
#include <unistd.h>
#include "arducam.h"

#define OV2640_CHIPID_HIGH  0x0A
#define OV2640_CHIPID_LOW   0x0B

void setup()
{
    uint8_t vid,pid;
    uint8_t temp;
    wiring_init();
    arducam(CAM1_CS,-1,-1,-1);
    
     // Check if the ArduCAM SPI bus is OK
    arducam_write_reg(ARDUCHIP_TEST1, 0x55, CAM1_CS);
    temp = arducam_read_reg(ARDUCHIP_TEST1, CAM1_CS);
    //printf("temp=%x\n",temp);
    if(temp != 0x55) {
        printf("SPI interface error!\n");
        exit(EXIT_FAILURE);
    }
     else{
    	   printf("SPI interface OK!\n");
    	}
    
    // Change MCU mode
    arducam_write_reg(ARDUCHIP_MODE, 0x00, CAM1_CS);
    
    // Check if the camera module type is OV2640
    arducam_i2c_read(OV2640_CHIPID_HIGH, &vid);
    arducam_i2c_read(OV2640_CHIPID_LOW, &pid);
    if((vid != 0x26) || (pid != 0x42)) {
		printf("%x:%x", pid,vid);
        printf("Can't find OV2640 module!\n");
        //exit(EXIT_FAILURE);
    } else {
        printf("OV2640 detected\n");
    }
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("Usage: %s filename\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

  	if (argc == 2) 
  	{
		setup();
		arducam_init();
		if (strcmp(argv[1], "160x120") == 0) arducam_set_jpeg_size(sz160x120);
		else if (strcmp(argv[1], "176x144") == 0) arducam_set_jpeg_size(sz176x144);
		else if (strcmp(argv[1], "320x240") == 0) arducam_set_jpeg_size(sz320x240);
		else if (strcmp(argv[1], "352x288") == 0) arducam_set_jpeg_size(sz352x288);
		else if (strcmp(argv[1], "640x480") == 0) arducam_set_jpeg_size(sz640x480);
		else if (strcmp(argv[1], "800x600") == 0) arducam_set_jpeg_size(sz800x600);
		else if (strcmp(argv[1], "1024x768") == 0) arducam_set_jpeg_size(sz1024x768);
		else if (strcmp(argv[1], "1280x960") == 0) arducam_set_jpeg_size(sz1280x960);
		else if (strcmp(argv[1], "1600x1200") == 0) arducam_set_jpeg_size(sz1600x1200);
		else 
		{
			printf("Error: unknown resolution.\n");
			exit(EXIT_FAILURE); 
		}
		//sleep(1); // Let auto exposure do it's thing after changing image settings
		printf("Changed resolution to %s\n", argv[1]);
		//delay(1000);
	} else {
	  printf("Error: unknown or missing argument.\n");
	  exit(EXIT_FAILURE);
	}
  exit(EXIT_SUCCESS);
}
