all :  ov2640_capture ov2640_4cams_capture arducam_init_and_jpeg_size arducam_spi_test

objects = arducam.o arducam_arch_raspberrypi.o 

arducam_init_and_jpeg_size : $(objects) arducam_init_and_jpeg_size.o
	gcc -o arducam_init_and_jpeg_size $(objects) arducam_init_and_jpeg_size.o -lwiringPi -Wall
arducam_spi_test : $(objects) arducam_spi_test.o
	gcc -o arducam_spi_test $(objects) arducam_spi_test.o -lwiringPi -Wall
ov2640_capture : $(objects) arducam_ov2640_capture.o 
	gcc -o ov2640_capture $(objects) arducam_ov2640_capture.o -lwiringPi -Wall 	
ov2640_4cams_capture : $(objects) arducam_ov2640_4cams_capture.o 
	gcc -o ov2640_4cams_capture $(objects) arducam_ov2640_4cams_capture.o -lwiringPi -Wall	

arducam_init_and_jpeg_size.o : arducam_init_and_jpeg_size.c
	gcc -c arducam_init_and_jpeg_size.c -lwiringPi -Wall
arducam_spi_test.o : arducam_spi_test.c
	gcc -c arducam_spi_test.c -lwiringPi -Wall
arducam.o : arducam.c
	gcc -c arducam.c -lwiringPi -Wall
arducam_arch_raspberrypi.o : arducam_arch_raspberrypi.c
	gcc -c arducam_arch_raspberrypi.c -lwiringPi -Wall


arducam_ov2640_capture.o : arducam_ov2640_capture.c
	gcc -c arducam_ov2640_capture.c -lwiringPi -Wall
arducam_ov2640_4cams_capture.o : arducam_ov2640_4cams_capture.c
	gcc -c arducam_ov2640_4cams_capture.c -lwiringPi -Wall	


clean : 
	rm -f  ov2640_capture ov2640_4cams_capture arducam_spi_test arducam_init_and_jpeg_size $(objects) *.o
