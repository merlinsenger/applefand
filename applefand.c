// Apple iMac CPU fan daemon
// Reads temperature sensors and controls the fan proportionally
// Temperatures are in milli-Celsius (except on standard output)
// Merlin Senger, December 2019

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

// Sensors can be found at
//
// - /sys/devices/platform/applesmc.768/
// - /sys/class/hwmon/hwmon0/
//
//
// We use:
//
// - TA0p: Ambient Temperature
// - TC0C: Central Processing Unit
// - TG0G: Graphical Processing Unit
// - TL0p: Liquid Crystal Display
// - TO0P: Optical Disk Drive
// - TS2P: ?
// - Tp2H: Power Supply
//
#define AMB "/sys/devices/platform/applesmc.768/temp3_input"    // TA0p, Ambient
#define CPU "/sys/devices/platform/applesmc.768/temp9_input"	// TC1C, Central Processing Unit
#define PWR "/sys/devices/platform/applesmc.768/temp34_input"   // Tp2H, Power Supply
#define SOU "/sys/devices/platform/applesmc.768/temp28_input"   // TS2P, South Bridge Proximity
#define GPU "/sys/devices/platform/applesmc.768/temp11_input"   // TG0H, Graphical Processing Unit Heatsink
#define ODD "/sys/devices/platform/applesmc.768/temp24_input"   // TO0P, Optical Disc Drive Proximity

// Fan sysfs output files
#define LEFT_FAN "/sys/devices/platform/applesmc.768/fan3_min"

// Maximum string buffer length (stores milli-Celsius)
#define BUFLEN 8

// Temperature and fan speed range
#define COLD 70000
#define HOT 83000
#define IDLE 940
#define SLOW 1200
#define FAST 2100

// Sleep time between fan speed adjustment
#define SLEEP 10

// Program output
#define FORMAT0 "applefand: Set speed between %i and %i rpm proportionally\n"
#define FORMAT1 "applefand: based on the temperature between %i°C and %i°C\n"
#define FORMAT2 "applefand: AMB %i°C, CPU %i°C, PWR %i°C, SOU %i°C, GPU %i°C, ODD %i°C, setting left fan to to %irpm\n"

// Main function with infinite loop

int readTemperature(char* file)
{
	FILE *fp;
	char str[BUFLEN];
	fp = fopen(file, "r");
	if (fp == NULL) {
		printf("Could not open file %s\n", file);
		return 1;
	}
	if (fgets(str, BUFLEN, fp) == NULL) {
		printf("Could not read from file %s\n", file);
		return 1;
	}
	fclose(fp);
	return atoi(str);
}

int main()
{
	FILE *fp;
	char str[BUFLEN];
	int speed, amb, cpu, pwr, sou, gpu, odd;
	int left_max, center_max, right_max;

	printf(FORMAT0, SLOW, FAST);
	printf(FORMAT1, COLD / 1000, HOT / 1000);

	while(1) {
		amb = readTemperature(AMB);
		cpu = readTemperature(CPU);
		pwr = readTemperature(PWR);
		sou = readTemperature(SOU);
		gpu = readTemperature(GPU);
		odd = readTemperature(ODD);

		left_max = pwr > cpu ? pwr : cpu;
		center_max = sou;
		right_max = gpu > odd ? gpu : odd;

		// Calculate speed proportionally, then cap to minimum and maximum
		speed = (FAST - SLOW)  *  (left_max - COLD) / (HOT - COLD)  +  SLOW;
		speed = speed < SLOW ? IDLE : speed;
		speed = speed > FAST ? FAST : speed;
		printf(FORMAT2, amb/1000, cpu/1000, pwr/1000, sou/1000, gpu/1000, odd/1000, speed);

		// Convert to string and write to fan speed sysfs file
		snprintf(str, BUFLEN, "%i", speed);
		fp = fopen(LEFT_FAN, "w");
		if (fp == NULL) {
			printf("Could not open file %s\n", LEFT_FAN);
			printf("Do you have sufficient priviliges (are you root?)\n");
			return 1;
		}
		fputs(str, fp);
		fclose(fp);

		// Sleep before continuing
		sleep(SLEEP);
	}

	return 1;
}
