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
#define LEFT_FAN   "/sys/devices/platform/applesmc.768/fan3_min"
#define CENTER_FAN "/sys/devices/platform/applesmc.768/fan2_min"
#define RIGHT_FAN  "/sys/devices/platform/applesmc.768/fan1_min"

// Maximum string buffer length (stores milli-Celsius)
#define BUFLEN 8

// Temperature and fan speed range
#define COLD 60000
#define HOT 83000
#define LEFT_IDLE 940
#define LEFT_SLOW 1200
#define LEFT_FAST 2100
#define CENTER_IDLE 1100
#define CENTER_SLOW 1300
#define CENTER_FAST 5500
#define RIGHT_IDLE 1000
#define RIGHT_SLOW 1200
#define RIGHT_FAST 3800

// Sleep time between fan speed adjustment
#define SLEEP 1

// Program output
#define FORMAT1 "applefand: Set speeds proportionally based on the temperature between %i°C and %i°C\n"
#define FORMAT2 "applefand: AMB %i°C, CPU %i°C, PWR %i°C, SOU %i°C, GPU %i°C, ODD %i°C, setting fans to %irpm %irpm %irpm\n"

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

void setFan(char* file, int speed) {
	FILE *fp;
	char str[BUFLEN];
	snprintf(str, BUFLEN, "%i", speed);
	fp = fopen(file, "w");
	if (fp == NULL) {
		printf("Could not open file %s\n", file);
		printf("Do you have sufficient priviliges (are you root?)\n");
		return;
	}
	fputs(str, fp);
	fclose(fp);
}

int calc(int temp, int hot, int cold, int idle, int slow, int fast) {
	int res = 0;
	res = (fast - slow)  *  (temp - cold) / (hot - cold)  +  slow;
	res = res < slow ? idle : res;
	res = res > fast ? fast : res;
}

int main()
{
	FILE *fp;
	char str[BUFLEN];
	int left_speed, center_speed, right_speed, amb, cpu, pwr, sou, gpu, odd;
	int left_max, center_max, right_max;

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

		left_speed   = calc(left_max, HOT, COLD, LEFT_IDLE, LEFT_SLOW, LEFT_FAST);
		center_speed = calc(center_max, HOT, COLD, CENTER_IDLE, CENTER_SLOW, CENTER_FAST);
		right_speed  = calc(right_max, HOT, COLD, RIGHT_IDLE, RIGHT_SLOW, RIGHT_FAST);

		// Calculate speed proportionally, then cap to minimum and maximum
		printf(FORMAT2, amb/1000, cpu/1000, pwr/1000, sou/1000, gpu/1000, odd/1000, left_speed, center_speed, right_speed);

		setFan(LEFT_FAN, left_speed);
		setFan(CENTER_FAN, center_speed);
		setFan(RIGHT_FAN, right_speed);

		// Sleep before continuing
		sleep(SLEEP);
	}

	return 1;
}
