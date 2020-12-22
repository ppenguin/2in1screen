// gcc -O2 -o 2in1screen 2in1screen.c

/*
	Modified by ppenguin:
	- removed number of states == 2 option
	- changed rotation detection algorithm to be more suitable for handheld/tabletop usage
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define DATA_SIZE 256

char basedir[DATA_SIZE];
char *basedir_end = NULL;
char content[DATA_SIZE];
char command[DATA_SIZE*4];

char *ROT[]   = {"normal", 				"inverted", 			"left", 				"right"};
char *COOR[]  = {"1 0 0 0 1 0 0 0 1",	"-1 0 1 0 -1 1 0 0 1", 	"0 -1 1 1 0 0 0 0 1", 	"0 1 0 -1 0 1 0 0 1"};
// char *TOUCH[] = {"enable", 				"disable", 				"disable", 				"disable"};

double accel_y = 0.0,
	   accel_x = 0.0,
	   accel_z = 0.0,
	   accel_th = 1.5,
	   accel_max = 10.0; // threshold offset value [m/s^2]

int current_state = 0;

/*  the original algorithm isn't practical for "in-hand tablet usage",
	for that we would need to be able to define a rotation threshold with
	a kind of angle offset.

	Or, better: the orientation *never changes* if display close to horiontal
	(table top), say z <=-9.0 
	Further rules:
	abs(x) < abs(y) ==> landscape, else portrait.
*/

int rotation_changed(){
	int state = 0;

	// printf("check rotation (current_state==%d): accel_x==%2.2f\taccel_y==%2.2f\taccel_z==%2.2f\n", current_state, accel_x, accel_y, accel_z);

	if(abs(accel_z) > abs(accel_max - accel_th)) {
		// printf("Ignoring rotation change because screen almost flat\n");
		return 0; // don't change because screen flat
	}

	if(abs(accel_x) < abs(accel_y)) { // landscape
		state = (accel_y < 0.0) ? 0 : 1;
		// printf("landscape: new state==%d\n", state);
	} else { // portrait
		state = (accel_x < 0.0) ? 3 : 2;
		// printf("portrait: new state==%d\n", state);
	}

	if(current_state!=state){
		current_state = state;
		return 1;
	}
	else return 0;
}

FILE* bdopen(char const *fname, char leave_open){
	*basedir_end = '/';
	strcpy(basedir_end+1, fname);
	FILE *fin = fopen(basedir, "r");
	setvbuf(fin, NULL, _IONBF, 0);
	fgets(content, DATA_SIZE, fin);
	*basedir_end = '\0';
	if(leave_open==0){
		fclose(fin);
		return NULL;
	}
	else return fin;
}

void rotate_screen(){
	// printf("rotating screen: %s\n", ROT[current_state]);
	sprintf(command, "xrandr -o %s", ROT[current_state]);
	system(command);
	sprintf(command, "xinput set-prop \"%s\" \"Coordinate Transformation Matrix\" %s", "pointer:ELAN2514:00 04F3:29F5", COOR[current_state]);
	system(command);
	sprintf(command, "xinput set-prop \"%s\" \"Coordinate Transformation Matrix\" %s", "pointer:ELAN2514:00 04F3:29F5 Pen (0)", COOR[current_state]);
	system(command);
}

int main(int argc, char const *argv[]) {
	FILE *pf = popen("ls /sys/bus/iio/devices/iio:device*/in_accel*", "r");
	if(!pf){
		fprintf(stderr, "IO Error.\n");
		return 2;
	}

	if(fgets(basedir, DATA_SIZE , pf)!=NULL){
		basedir_end = strrchr(basedir, '/');
		if(basedir_end) *basedir_end = '\0';
		fprintf(stderr, "Accelerometer: %s\n", basedir);
	}
	else{
		fprintf(stderr, "Unable to find any accelerometer.\n");
		return 1;
	}
	pclose(pf);

	bdopen("in_accel_scale", 0);
	double scale = atof(content);

	FILE *dev_accel_y = bdopen("in_accel_y_raw", 1);
	FILE *dev_accel_x = bdopen("in_accel_x_raw", 1);
	FILE *dev_accel_z = bdopen("in_accel_z_raw", 1);

	while(1){
		fseek(dev_accel_y, 0, SEEK_SET);
		fgets(content, DATA_SIZE, dev_accel_y);
		accel_y = atof(content) * scale;

		fseek(dev_accel_x, 0, SEEK_SET);
		fgets(content, DATA_SIZE, dev_accel_x);
		accel_x = atof(content) * scale;

		fseek(dev_accel_z, 0, SEEK_SET);
		fgets(content, DATA_SIZE, dev_accel_z);
		accel_z = atof(content) * scale;

		if(rotation_changed())
			rotate_screen();
		sleep(2);
	}
	
	return 0;
}
