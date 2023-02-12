// Apple iMac CPU fan daemon
// Reads temperature sensors and controls the fan proportionally
// Temperatures are in milli-Celsius (except on standard output)
// Merlin Senger, December 2019

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

// Sensor sysfs input files
#define T0 "/sys/class/hwmon/hwmon0/temp2_input"
#define T1 "/sys/class/hwmon/hwmon0/temp3_input"

// Number of sensors
#define SENSORS 2

// Fan sysfs output files
#define FAN "/sys/devices/platform/applesmc.768/fan3_min"

// Maximum string buffer length (stores milli-Celsius)
#define BUFLEN 8

// Temperature and fan speed range
#define COLD 50000
#define HOT 75000
#define SLOW 940
#define FAST 4500

// Sleep time between fan speed adjustment
#define SLEEP 5

// Program output
#define FORMAT0 "applefand: Set speed between %i and %i rpm proportionally\n"
#define FORMAT1 "applefand: based on the temperature between %i°C and %i°C\n"
#define FORMAT2 "applefand: CPU temperature is %i°C, setting speed to %irpm\n"

// Main function with infinite loop
int main()
{
    FILE *fp;
    const char *files[SENSORS] = { T0, T1 };
    char str[BUFLEN];
    int i, speed, celsius;

    printf(FORMAT0, SLOW, FAST);
    printf(FORMAT1, COLD / 1000, HOT / 1000);

    while(1) {
      // Read each sensor from sysfs and accumulate an average
      celsius=0;
      for (i = 0; i < SENSORS; i++) {
          fp = fopen(files[i], "r");
          if (fp == NULL) {
              printf("Could not open file %s\n",files[i]);
              return 1;
          }
          if (fgets(str, BUFLEN, fp) == NULL) {
              printf("Could not read from file %s\n",files[i]);
              return 1;
          }
          celsius += atoi(str) / SENSORS;
          fclose(fp);
      }

      // Calculate speed proportionally, then cap to minimum and maximum
      speed = (FAST - SLOW)  *  (celsius - COLD) / (HOT - COLD)  +  SLOW;
      speed = speed < SLOW ? SLOW : speed;
      speed = speed > FAST ? FAST : speed;
      printf(FORMAT2, celsius / 1000, speed);

      // Convert to string and write to fan speed sysfs file
      snprintf(str, BUFLEN, "%i", speed);
      fp = fopen(FAN, "w");
      if (fp == NULL) {
          printf("Could not open file %s\n",FAN);
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
