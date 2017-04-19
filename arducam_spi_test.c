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

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		errno = EINVAL;
		perror("Wrong argument, only send CS");
		return -1;
	}
	else
	{
		int CS = atoi(argv[1]);
		uint8_t temp;
		
		wiring_init();
		arducam(CS,-1,-1,-1);
		
		arducam_write_reg(ARDUCHIP_TEST1, 0x55, CS);
		temp = arducam_read_reg(ARDUCHIP_TEST1, CS);
		return (temp != 0x55) ? -1 : 0;
	}
}
