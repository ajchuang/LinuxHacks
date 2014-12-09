#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "gpsd.h"

#define GPSD_DEBUG

#ifdef GPSD_DEBUG
#define log	printf
#else
#define log(...)
#endif

static int read_gpsloc(FILE *fd, struct gps_location *loc)
{
	if (fd == NULL || loc == NULL) {
		log("Null FILE or gps_location in read_gpsloc");
		return -1;
	}
	fscanf(fd, "%lf", &loc->latitude);
	fscanf(fd, "%lf", &loc->longitude);
	fscanf(fd, "%f", &loc->accuracy);

	log("Read GPS_LOCATION_FILE latitude: %f longitude: %f accuracy: %f\n",
			loc->latitude, loc->longitude, loc->accuracy);

	return 1;
}

static void daemonize(void)
{
	pid_t pid = 0;
	int fd = 0;

	/* Authentication */
	if (getuid() != 0) {
		log("Sorry you are not root\n");
		exit(EXIT_FAILURE);
	}

	/* Fork to create new session */
	pid = fork();
	if (pid < 0) {
		log("Failed to new session\n");
		exit(EXIT_FAILURE);
	} else if (pid > 0) {
		log("1st parent to terminate\n");
		exit(EXIT_SUCCESS);
	}

	if (setsid() < 0) {
		log("Failed to set sid\n");
		exit(EXIT_FAILURE);
	}

	/* Fork to get rid of TTY */
	pid = fork();
	if (pid < 0) {
		log("Failed to fork\n");
		exit(EXIT_FAILURE);
	} else if (pid > 0) {
		log("nst parent to terminate\n");
		exit(EXIT_SUCCESS);
	}

	/* Set up directory */
	if (chdir("/") < 0) {
		log("Failed to change dir\n");
		exit(EXIT_FAILURE);
	}

	umask(0);

	/* Turn of std io */
	fd = open("/dev/null", O_RDWR);
	if (fd < 0) {
		log("Failed to open null dev\n");
		exit(EXIT_FAILURE);
	}
	dup2(fd, 0);
#ifndef GPSD_DEBUG
	dup2(fd, 1);
#endif
	dup2(fd, 2);
	close(fd);
}

int main(int argc, char *argv[])
{
	FILE *fd = NULL;
	struct gps_location loc;

	daemonize();

	while (1) {
		fd = fopen(GPS_LOCATION_FILE, "r");
		if (fd == NULL) {
			log("Failed to open GPS_LOCATION_FILE\n");
			exit(EXIT_FAILURE);
		}

		if (read_gpsloc(fd, &loc) < 0) {
			log("Failed to read_gpsloc\n");
			fclose(fd);
			exit(EXIT_FAILURE);
		}

		if (set_gps_location(&loc) < 0)
			log("Failed to set_gps_location\n");

		/*read the values once every second*/
		sleep(1);
		fclose(fd);
	}

	return 0;
}
