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

#define BUF_SIZE (384*1024)
uint8_t buffer[BUF_SIZE] = {0xFF};
void setup(int CS)
{
	wiring_init();
	arducam(CS,-1,-1,-1);
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("Usage: %s filename\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

  	if (argc == 3) 
  	{
		int arduCams[] = { 21, 22, 23, 27 };
		int CS = arduCams[atoi(argv[2])];
		setup(CS);
      
      // Flush the FIFO
      arducam_flush_fifo(CS);    
      // Clear the capture done flag
      arducam_clear_fifo_flag(CS);
      // Start capture
      printf("Start capture\n");  
      arducam_start_capture(CS);
      while (!(arducam_read_reg(ARDUCHIP_TRIG,CS) & CAP_DONE_MASK)) ;
      printf("CAM1 Capture Done\n");
              
       // Open the new file
      FILE *fp1 = fopen(argv[1], "w+");   
      if (!fp1) {
          printf("Error: could not open %s\n", argv[1]);
          exit(EXIT_FAILURE);
      }
       
      printf("Reading FIFO\n");    
      size_t len = read_fifo_length(CS);
      if (len >= 393216){
		   printf("Over size.");
		    exit(EXIT_FAILURE);
		  }else if (len == 0 ){
		    printf("Size is 0.");
		    exit(EXIT_FAILURE);
		  } 
		  digitalWrite(CS,LOW);  //Set CS low       
      set_fifo_burst(BURST_FIFO_READ);
      arducam_spi_transfers(buffer,1);//dummy read  
      int32_t i=0;
      while(len>4096)
      {	 
      	arducam_transfers(&buffer[i],4096);
      	len -= 4096;
      	i += 4096;
      }
      arducam_spi_transfers(&buffer[i],len); 

      fwrite(buffer, len+i, 1, fp1);
      digitalWrite(CS,HIGH);  //Set CS HIGH
       //Close the file
      delay(100);
      fclose(fp1);  
      // Clear the capture done flag
      arducam_clear_fifo_flag(CS);

  } else {
      printf("Error: unknown or missing argument.\n");
      exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}
