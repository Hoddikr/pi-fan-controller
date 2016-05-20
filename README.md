# pi-fan-controller
A simple daemon to control a 5v fan. Made in collaboration with Björn Eiríksson (www.einhugur.com).

This daemon controls a 5v fan based on the current cpu temperature. This was developed using an orange pi PC 
running Armbian Debian GNU/Linux 8 (jessie). The daemon controls the fan speed via a circuit design by Björn
and the whole process can be seen here on his blog here:

<INSERT BLOG LINK>

========================
Building and installing
========================
Clone the repo and run:
1. make
2. sudo make install

This compiles and copies the executable and config files. This daemon is managed via upstart so the configuration
file drops into /etc/init.

========================
Configuring and starting
========================
The configuration file is pretty self-explanatory and is located here at /etc/init/pi-fan-controller.conf.

You can configure the following parameters:
* The cpu temperature levels at which to start the fan and increase the fan speed.
* The interval in seconds to check the cpu temperature.
* The log level. This daemon logs into /var/log/syslog
* The time in seconds the fan should keep spinning after the lowest target temperature