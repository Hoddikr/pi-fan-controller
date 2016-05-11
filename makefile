INCLUDE = -I/usr/local/include
LIBFOLDER = -L/usr/local/lib
LIBS = -lwiringPi -lwiringPiDev -lpthread -lm

SRC = pi-fan-controller.cpp
EXENAME = pi-fan-controller

all: 
	g++ -std=c++11 $(INCLUDE) $(LIBFOLDER) $(LIBS) $(SRC) -o $(EXENAME)

install:
	cp ./pi-fan-controller /usr/sbin -f
	cp ./upstart/pi-fan-controller.conf /etc/init -f