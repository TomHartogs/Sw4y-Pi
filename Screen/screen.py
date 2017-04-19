import atexit
import io
import os
import picamera
import pygame
import yuv2rgb
import wiringpi
import time
import fnmatch
import imghdr
import requests
from pygame.locals import *
from subprocess import check_call, CalledProcessError, Popen, PIPE
from screenInterface import *

initAndJPGSizePath 	= "/kiekje/Code/arducam_init_and_jpeg_size"
SPITestFilePath 	= "/kiekje/Code/arducam_spi_test"
takePictureFilePath 	= "/kiekje/Code/ov2640_4cams_capture"
picturePath 		= "/kiekje/"

SCREEN_WIDTH 	= 320
SCREEN_HEIGHT 	= 240

screenMode = 1

icons = [] # This list gets populated at startup
# Load all icons at startup.
for file in os.listdir(iconPath):
  if fnmatch.fnmatch(file, '*.png'):
    icons.append(Icon(file.split('.')[0]))

def doneCallback():
	global screenMode
	screenMode = 1
	print("done")
	
MAX_ERRORS = 4
ERROR_OFFSET = 20
ERROR_SIZE = (SCREEN_WIDTH - MAX_ERRORS * ERROR_OFFSET) / MAX_ERRORS
ERROR_POS_Y = (SCREEN_HEIGHT / 2) - (ERROR_SIZE / 2)

def calculateOffset(nr):
	offset = ERROR_OFFSET/2
	offset += nr * (ERROR_SIZE + ERROR_OFFSET)
	return offset

buttons = [
	# Screenmode testing
	[Button((calculateOffset(0), ERROR_POS_Y, ERROR_SIZE, ERROR_SIZE), bg='unknown'),
	Button((calculateOffset(1), ERROR_POS_Y, ERROR_SIZE, ERROR_SIZE), bg='unknown'),
	Button((calculateOffset(2), ERROR_POS_Y, ERROR_SIZE, ERROR_SIZE), bg='unknown'),
	Button((calculateOffset(3), ERROR_POS_Y, ERROR_SIZE, ERROR_SIZE), bg='unknown'),
	Button((0,0,320, 240), cb=doneCallback)],
	
	# Screenmode started
	[]
]

for s in buttons:        # For each screenful of buttons...
  for b in s:            #  For each button on screen...
    for i in icons:      #   For each icon...
      if b.bg == i.name: #    Compare names; match?
        b.iconBg = i     #     Assign Icon to Button
        b.bg     = None  #     Name no longer used; allow garbage collection
      if b.fg == i.name:
        b.iconFg = i
        b.fg     = None

# Init framebuffer/touchscreen environment variables
os.putenv('SDL_VIDEODRIVER', 'fbcon')
os.putenv('SDL_FBDEV'      , '/dev/fb1')
os.putenv('SDL_MOUSEDRV'   , 'TSLIB')
os.putenv('SDL_MOUSEDEV'   , '/dev/input/touchscreen')

# Buffers for viewfinder data
rgb = bytearray(SCREEN_WIDTH * SCREEN_HEIGHT * 3)
yuv = bytearray(SCREEN_WIDTH * SCREEN_HEIGHT * 3 / 2)

# Init pygame and screen
pygame.init()
pygame.mouse.set_visible(False)
screen = pygame.display.set_mode((0,0), pygame.FULLSCREEN)

CAM_CS = [21, 22, 23, 27]

# Init camera and set up default values
camera = picamera.PiCamera()
atexit.register(camera.close)
camera.resolution = (SCREEN_WIDTH, SCREEN_HEIGHT)	

def testCamera(CS):
	try:
		check_call([SPITestFilePath, str(CS)])
		buttons[0][CAM_CS.index(CS)].value=True
		for icon in icons:
			if icon.name == 'succes':
				buttons[0][CAM_CS.index(CS)].iconBg=icon
	except CalledProcessError:
		print ("CAM{CS} test failed".format(CS=CS))
		buttons[0][CAM_CS.index(CS)].value=False
		for icon in icons:
			if icon.name == 'error':
				buttons[0][CAM_CS.index(CS)].iconBg=icon

# Scan files in a directory, locating JPEGs with names matching the
# software's convention (IMG_XXXX.JPG), returning a tuple with the
# lowest and highest indices (or None if no matching files).
def imgRange(path):
	min = 9999
	max = 0
	try:
	  for file in os.listdir(path):
	    if fnmatch.fnmatch(file, 'IMG_[0-9][0-9][0-9][0-9]-[1-4].jpg'):
	      i = int(file[4:8])
	      if(i < min): min = i
	      if(i > max): max = i
	finally:
	  return None if min > max else (min, max)


def takePicture ():
	interrupt_time = int(round(time.time() * 1000))
	if (interrupt_time - takePicture.last_interrupt_time > 200):
		for CS in CAM_CS:
			testCamera(CS)
		AllCamerasConnected = True
		for button in buttons[0]:
			if(button.value == False):
				AllCamerasConnected = False
				
		if(AllCamerasConnected == True):
			photosTakenSuccesfull = True
			print("*Click*")
			saveId = 0
			r = imgRange(picturePath)
			if r is None:
				saveId = 1
			else:
				saveId = r[1] + 1
			if saveId > 9999: saveId = 0
			pictureName = 'IMG_' + '%04d' % saveId
			check_call(["sudo", takePictureFilePath, picturePath + pictureName ])
			
			for CS in CAM_CS:
				if(imghdr.what(picturePath + pictureName + '-' + str(CAM_CS.index(CS)+1) + ".jpg") != "jpeg"):
					photosTakenSuccesfull = False
			
			if(photosTakenSuccesfull):
				url = 'http://kiekje.local/kiekje'
				headers = { 'cache-control': 'no-cache', 'content-type': 'application/x-www-form-urlencoded' }
				payload = {'p1': pictureName + '-1.jpg', 'p2': pictureName + '-2.jpg', 'p3': pictureName + '-3.jpg', 'p4': pictureName + '-4.jpg'}
				r = requests.post(url, headers=headers, data=payload)
				print(r)
		else:
			global screenMode
			screenMode = 0
			
	takePicture.last_interrupt_time = interrupt_time;
	return;

def reset():
	Popen("sudo rm /kiekje/*.jpg", shell=True, stdin=PIPE, stdout=PIPE, stderr=PIPE, close_fds=True)
	Popen("sudo rm /kiekje/kiekjes.db", shell=True, stdin=PIPE, stdout=PIPE, stderr=PIPE, close_fds=True)
	Popen("sudo reboot", shell=True, stdin=PIPE, stdout=PIPE, stderr=PIPE, close_fds=True)

triggerButton = 26
resetButton = 17

wiringpi.wiringPiSetupGpio()
wiringpi.pinMode(triggerButton, wiringpi.GPIO.INPUT)
wiringpi.pullUpDnControl(triggerButton, wiringpi.GPIO.PUD_UP)
takePicture.last_interrupt_time = 0


wiringpi.pinMode(resetButton, wiringpi.GPIO.INPUT)
wiringpi.pullUpDnControl(resetButton, wiringpi.GPIO.PUD_UP)
wiringpi.wiringPiISR(resetButton, wiringpi.GPIO.INT_EDGE_FALLING, reset)

check_call(["sudo", initAndJPGSizePath, '1600x1200' ])

buttonState = 1
lastButtonState = 1

while(True):
	for event in pygame.event.get():
		if(event.type is MOUSEBUTTONDOWN):
			pos = pygame.mouse.get_pos()
			print(pos)
			for b in buttons[screenMode]:
				if b.selected(pos): break
				
	if(screenMode == 0):
		for CS in CAM_CS:
			testCamera(CS)
		print(screenMode)
		screen.fill(0)
		time.sleep(0.1)
		
	elif(screenMode == 1):
		buttonState = wiringpi.digitalRead(triggerButton)
		if(buttonState != lastButtonState):
			if(buttonState == wiringpi.LOW):
				takePicture()
		lastButtonState = buttonState
		stream = io.BytesIO() # Capture into in-memory stream
		camera.capture(stream, use_video_port=True, format='raw')
		stream.seek(0)
		stream.readinto(yuv)  # stream -> YUV buffer
		stream.close()
		yuv2rgb.convert(yuv, rgb, SCREEN_WIDTH, SCREEN_HEIGHT)
		img = pygame.image.frombuffer(rgb[0:(SCREEN_WIDTH * SCREEN_HEIGHT * 3)],(SCREEN_WIDTH, SCREEN_HEIGHT), 'RGB')
	
		if img:
			screen.blit(img,(0,0))
			
	for i,b in enumerate(buttons[screenMode]):
		b.draw(screen)
		
	pygame.display.update()
