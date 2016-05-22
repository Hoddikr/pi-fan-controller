INCLUDE = -I/usr/local/include
LIBFOLDER = -L/usr/local/lib
LIBS = -lwiringPi -lwiringPiDev -lpthread -lm

SRC = pi-fan-controller.cpp
EXENAME = pi-fan-controller

all: 
	g++ -std=c++11 $(INCLUDE) $(LIBFOLDER) $(LIBS) $(SRC) -o $(EXENAME)

install.upstart:
	cp ./pi-fan-controller /usr/bin -f
	cp ./upstart/pi-fan-controller.conf /etc/init -f

install.systemd:
	cp ./pi-fan-controller /usr/bin -f
	cp ./systemd/pi-fan-controller.service /etc/systemd/system -f
	cp ./systemd/pi-fan-controller.env /etc -f