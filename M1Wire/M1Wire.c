//  read from MaxDetect 1-Wire bus from GPIO on the Raspberry-Pi
//  Makarenko Alexander
//  04.06.2013

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <bcm2835.h>

#define DHT11 1
#define DHT22 2
#define debug 1
#define simple 2

unsigned char MaxDetect1Wire(unsigned char type, unsigned char pin, unsigned char param);

unsigned char main(unsigned char argc, unsigned char **argv) {
	if (!bcm2835_init()) return 1;

	if (argc < 3 || argc > 4) {
		printf("usage: %s DHT11|DHT22 GPIOpin# [debug|simple]\n", argv[0]);
		printf("example: %s DHT22 4 simple - Read from an DHT22 connected to GPIO #4 with simple output\n", argv[0]);
		return 2;
	}

	unsigned char type = 0;
	if (strcmp(argv[1], "DHT11") == 0) type = DHT11;
	else if (strcmp(argv[1], "DHT22") == 0) type = DHT22;
	else {
		printf("Select DHT11 or DHT22 as type!\n");
		return 3;
	}
  
	unsigned char pin = atoi(argv[2]);
	if (pin <= 0 || pin > 25) {
		printf("Please select a valid GPIO pin #\n");
		return 3;
	}

	unsigned char param = 0;
	if (argc == 4) {
		if (strcmp(argv[3], "debug") == 0) param = debug;
		else if (strcmp(argv[3], "simple") == 0) param = simple;
	}

	if (param == debug) printf("Using pin #%d\n", pin);

	MaxDetect1Wire(type, pin, param);
	return 0;
}

unsigned char  bitDetect(unsigned char pin, unsigned char bitTest, unsigned short maxTime) {
	unsigned short counter = 0;
	while (bcm2835_gpio_lev(pin) == bitTest) {
		delayMicroseconds(1);
		counter++;
		if (counter == maxTime) return 0;
	}
	return counter;
}

unsigned char MaxDetect1Wire(unsigned char type, unsigned char pin, unsigned char param) {
	unsigned char bits[40];
	unsigned char data[5] = {0,0,0,0,0}; 
	unsigned char bitid = 0;

	bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
	if (bcm2835_gpio_lev(pin) == 0 && bitDetect(pin, 0, 10000) == 0) return 0;

	bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(pin, 0);
	delay(18);
	bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
	
	if (bitDetect(pin, 1, 100) == 0) return 0;
	if (bitDetect(pin, 0, 100) == 0) return 0;
	if (bitDetect(pin, 1, 100) == 0) return 0;

	for (unsigned char i=0; i<40; i++) {
		if (bitDetect(pin, 0, 100) == 0) return 0;
		bits[i] = bitDetect(pin, 1, 100);
		if (bits[i] == 0) return 0;
		bitid++;
	}

	if (bitid == 40) {
		unsigned char j=0;
		for (int i=0; i<bitid; i++) {
			data[j/8] <<= 1;
			if (bits[i] > 40) data[j/8] |= 1;
			j++;
			if (param == debug) printf("bit %d: %d ms\n", i, bits[i]);
		}

		if (param == debug) printf("Data (%d): 0x%x 0x%x 0x%x 0x%x 0x%x\n", j, data[0], data[1], data[2], data[3], data[4]);

		if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
			if (type == DHT11) {
				if (param == simple) printf("%d.%d %d.%d\n", data[2], data[3], data[0], data[1]);
				else printf("Temperature = %d.%d *C Humidity = %d.%d %%\n", data[2], data[3], data[0], data[1]);
			}
			else if (type == DHT22) {
				float f, h;
				h = data[0] * 256 + data[1];
				h /= 10;
				f = (data[2] & 0x7F)* 256 + data[3];
				f /= 10.0;
				if (data[2] & 0x80)  f *= -1;
				if (param == simple) printf("%.1f %.1f\n", f, h);
				else printf("Temperature = %.1f *C Humidity = %.1f %%\n", f, h);
			}
			return 1;
		}
		else if (param == debug) {
			printf("CRC doesn't match\n");
		}
	}
	return 0;
}
