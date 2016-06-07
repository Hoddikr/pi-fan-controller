// A simple daemon that monitors CPU temperature and controls a 5v fan
// Based on code from http://shahmirj.com/blog/beginners-guide-to-creating-a-daemon-in-linux

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <iostream>
#include <cstdio>
#include <string>
#include <memory>
#include <wiringPi.h>
#include <chrono>

using namespace std;
using namespace chrono;

#define DAEMON_NAME "pi-fan-controller"

// Fan state
#define FAN_STOPPED 0
#define FAN_STOPPING 1 
#define FAN_SPEED_1 2
#define FAN_SPEED_2 3
#define FAN_SPEED_3 4
#define FAN_SPEED_4 5
#define FAN_SPEED_5 6

char* message;

// GPIO pins to use
// Current implementation uses BCM numbers
int _pin1 = 2;
int _pin2 = 3;
int _pin3 = 4;

// Temperature thresholds
int _temp1 = 55;
int _temp2 = 60;
int _temp3 = 65;
int _temp4 = 70;
int _temp5 = 75;

// How often (in seconds) to check for cpu temp
int _checkInterval = 5;

int _logLevel = LOG_NOTICE;

int _stopWaitTime = 120;
steady_clock::time_point _stopWaitTimeStart;

int _currentState = FAN_STOPPED;

// Indicates wether this program should only stop the fans. This needs to be set with the -stopfan flag
// at startup which will stop the fan based on the given GPIO pin numbers
bool _stopFanOnly = false;

// Initializes the daemon settings based on the startup parameters given
void init(int argc, char *argv[])
{
	for(int i = 1; i < argc; i++)
	{
		if(i + 1 != argc)
		{		
			// GPIO pins
			if(std::string(argv[i]) == "-p1")
			{
				_pin1 = atoi(argv[i + 1]);		
			}
			else if (std::string(argv[i]) == "-p2")
			{
				_pin2 = atoi(argv[i + 1]);
			}
			else if (std::string(argv[i]) == "-p3")
			{
				_pin3 = atoi(argv[i +1]);
			}
			// Interval
			else if (std::string(argv[i]) == "-i")
			{
				_checkInterval = atoi(argv[i + 1]);
			}
			// Temperature thresholds
			else if (std::string(argv[i]) == "-t1")
			{
				_temp1 = atoi(argv[i + 1]);
			}
			else if (std::string(argv[i]) == "-t2")
			{
				_temp2 = atoi(argv[i + 1]);
			}
			else if (std::string(argv[i]) == "-t3")
			{
				_temp3 = atoi(argv[i + 1]);
			}
			else if (std::string(argv[i]) == "-t4")
			{
				_temp4 = atoi(argv[i + 1]);
			}
			else if (std::string(argv[i]) == "-t5")
			{
				_temp5 = atoi(argv[i + 1]);
			}	
			else if (std::string(argv[i]) == "-st")
			{			
				_stopWaitTime = atoi(argv[i + 1]);
			}		
			else if (std::string(argv[i]) == "-stopfan")
			{			
				_stopFanOnly = true;
			}				
			else if (std::string(argv[i]) == "-l")
			{				
				int logLevel = atoi(argv[i + 1]);
				
				if(logLevel >= 0 && logLevel <= 7)
				{
					_logLevel = logLevel;				
				}
				else
				{
					_logLevel = LOG_NOTICE;
				}
			}
		}
	}	
}

// Toggles pin state based on the given state
void togglePinState(int state)
{
	switch(state)
	{
		case FAN_STOPPED :
			syslog (LOG_INFO, ("Stoppin fan..."));
			digitalWrite(_pin1, LOW);
			digitalWrite(_pin2, LOW);
			digitalWrite(_pin3, LOW);		
			break;

		case FAN_SPEED_1 :
			syslog (LOG_INFO, ("Setting fan to speed 1..."));
			digitalWrite(_pin1, HIGH);
			digitalWrite(_pin2, LOW);
			digitalWrite(_pin3, LOW);
			break;
		
		case FAN_SPEED_2 :
			syslog (LOG_INFO, ("Setting fan to speed 2..."));
			digitalWrite(_pin1, LOW);
			digitalWrite(_pin2, HIGH);
			digitalWrite(_pin3, LOW);
			break;
			
		case FAN_SPEED_3 :
			syslog (LOG_INFO, ("Setting fan to speed 3..."));
			digitalWrite(_pin1, HIGH);
			digitalWrite(_pin2, LOW);
			digitalWrite(_pin3, HIGH);
			break;
		
		case FAN_SPEED_4 :
			syslog (LOG_INFO, ("Setting fan to speed 4..."));
			digitalWrite(_pin1, HIGH);
			digitalWrite(_pin2, HIGH);
			digitalWrite(_pin3, LOW);
			break;
			
		case FAN_SPEED_5 :
			syslog (LOG_INFO, ("Setting fan to speed 5..."));
			digitalWrite(_pin1, HIGH);
			digitalWrite(_pin2, HIGH);
			digitalWrite(_pin3, HIGH);
			break;
	}
}

// Executes a console command and returns the value
std::string exec(const char* cmd)
{
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
	{
		return "ERROR";
	}

    char buffer[128];
    std::string result = "";

	while (!feof(pipe.get()))
	{
        if (fgets(buffer, 128, pipe.get()) != NULL)
		{
            result += buffer;
		}
    }
    return result;
}

int getCPUTemp()
{
	FILE *temperatureFile;
	double T;
	temperatureFile = fopen ("/sys/class/thermal/thermal_zone0/temp", "r");
	
	if (temperatureFile == NULL)
	{
		syslog(LOG_PERROR, "Could not read /sys/class/thermal/thermal_zone0/temp");
		return 0;
	}
	
	fscanf (temperatureFile, "%lf", &T);
	fclose (temperatureFile);
	
	return (int)T;
}

//
void process()
{

	//syslog (LOG_DEBUG, ("CPU temp: " + exec("cat /sys/class/thermal/thermal_zone0/temp")).c_str());
	syslog (LOG_DEBUG, "CPU temp: " + getCPUTemp());
	
	int curTemp = getCPUTemp();

	int nextState = _currentState;

	// Turn off the fan
	if(curTemp < _temp1)
	{
		// Check if we are about to enter the waiting period or if we can simply turn off the fan
		if(_currentState > FAN_STOPPING)
		{
			nextState = FAN_STOPPING;
			_stopWaitTimeStart = steady_clock::now();
			
			// Set the speed to the lowest setting while winding down
			togglePinState(FAN_SPEED_1);
		}
		else if(_currentState == FAN_STOPPING)
		{
			// Check if the wait period has expired			
			duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - _stopWaitTimeStart);
			
			if(time_span.count() >= _stopWaitTime)
			{
				nextState = FAN_STOPPED;
			}
		}
		else
		{
			nextState = FAN_STOPPED;
		}
	}
	else if (curTemp >= _temp1 && curTemp < _temp2)
	{
		nextState = FAN_SPEED_1;
	}
	else if (curTemp >= _temp2 && curTemp < _temp3)
	{
		nextState = FAN_SPEED_2;
	}
	else if (curTemp >= _temp3 && curTemp < _temp4)
	{
		nextState = FAN_SPEED_3;
	}
	else if (curTemp >= _temp4 && curTemp < _temp5)
	{
		nextState = FAN_SPEED_4;
	}
	else if (curTemp >= _temp5)
	{
		nextState = FAN_SPEED_5;
	}

	// State changed so we need to change the GPIO pin output
	if (nextState != _currentState)
	{
		_currentState = nextState;
		
		// No need to toggle fan if we are still waiting
		if(_currentState != FAN_STOPPING)
		{
			togglePinState(_currentState);
		}
	}
}

int main(int argc, char *argv[])
{

	init(argc, argv);

    //Set our Logging Mask and open the Log    
	setlogmask(LOG_UPTO(_logLevel));
    openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
	
    syslog(LOG_INFO, _stopFanOnly ? "Shutting down the fan only" : "Entering Daemon");
	
    //----------------
    //Main Process
    //----------------
	
	// Use BCM numbering
	wiringPiSetupGpio();
	pinMode(_pin1, OUTPUT);
	pinMode(_pin2, OUTPUT);
	pinMode(_pin3, OUTPUT);
	
	// This will shut down the fan
	togglePinState(_currentState);

	if (!_stopFanOnly)
	{
		while(true)
		{
			process();
			sleep(_checkInterval);
		}
	}
    //Close the log
    closelog ();
}
