#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <wiringPiSPI.h>
#include <unistd.h>
#include <errno.h>
#include "arducam.h"

#define BUF_SIZE (384*1024)

void setup()
{
    wiring_init();
    arducam(CAM1_CS,CAM2_CS,CAM3_CS,CAM4_CS);
}

int main(int argc, char *argv[])
{   
	if(argc != 2)
	{
		errno = EINVAL;
		perror("Wrong argument, use filename only");
		return -1;
	}
	
	setup();

	int amountOfCameras = 4;
	int cameras[] = {CAM1_CS, CAM2_CS, CAM3_CS, CAM4_CS};

	int i = 0;
	for(; i < amountOfCameras; i++)
	{
		// Flush the FIFO
		arducam_flush_fifo(cameras[i]);    
		// Clear the capture done flag
		arducam_clear_fifo_flag(cameras[i]);
		// Start capture
		printf("Start capture\n");  
		arducam_start_capture(cameras[i]);
	}          
       
    i = 0; 
	size_t lengths[4];	  		
	uint8_t buffer[BUF_SIZE] = {0xFF};
	FILE *filePointers[amountOfCameras];
	for (; i < amountOfCameras; i++)
	{
		char fileName[20];
		sprintf(fileName, "%s-%i.jpg", argv[1], i+1);
		filePointers[i] = fopen(fileName, "w+");
		if(!filePointers[i])
		{
			printf("Error: could not open %s\n", fileName);
			exit(EXIT_FAILURE);
		}
		while (!(arducam_read_reg(ARDUCHIP_TRIG,cameras[i]) & CAP_DONE_MASK)) ;
		
		lengths[i] = read_fifo_length(cameras[i]);
		if(lengths[i] >= 393216)
		{
			printf("Over size.");
			exit(EXIT_FAILURE);
		}
		if(lengths[i] == 0) 
			printf("Size1 is 0."); 
			
		int bufferIndex = 0;
		printf("Reading file%i\n", i);
		digitalWrite(cameras[i],LOW);  //Set CS1 low       
		set_fifo_burst(BURST_FIFO_READ);
		arducam_spi_transfers(buffer,1);//dummy read  
		while(lengths[i]>4096)
		{	 
		arducam_transfers(&buffer[bufferIndex],4096);
		lengths[i] -= 4096; bufferIndex += 4096;
		}
		arducam_spi_transfers(&buffer[bufferIndex],lengths[i]); 
		printf("Done reading file%i\n", i);
		fwrite(buffer, lengths[i]+bufferIndex, 1, filePointers[i]);
		digitalWrite(cameras[i],HIGH);  //Set CS1 HIGH
		
		fclose(filePointers[i]);
		
		arducam_clear_fifo_flag(cameras[i]);
	}
    exit(EXIT_SUCCESS);
}
