# pi-fan-controller
A simple daemon to control a 5v fan. Made in collaboration with Björn Eiríksson (www.einhugur.com).

This daemon controls a 5v fan based on the current cpu temperature. This was developed using an orange pi PC 
running Armbian Debian GNU/Linux 8 (jessie). The daemon controls the fan speed via a circuit design by Björn
and the whole process can be seen here on his blog here:

<INSERT BLOG LINK>

========================
Building and installing
========================
1. Clone the repo and run:
```
make
```

2. For distros with upstart:
```
sudo make install.upstart
```

3. For distros with systemd:
```
sudo make install.systemd
```

This compiles and copies the executable and config files for the appropriate init system.

For upstart the config file goes here:
```
/etc/init/pi-fan-controller.conf
```

For systemd the unit- and environment file:
```
/etc/systemd/system/pi-fan-controller.service
/etc/pi-fan-controller.env
```

========================
Configuring and starting
========================
You can configure the following parameters in upstart/systemd files:
* The cpu temperature levels at which to start the fan and increase the fan speed.
* The interval in seconds to check the cpu temperature.
* The log level. This daemon logs into /var/log/syslog
* The time in seconds the fan should keep spinning after the lowest target temperature
