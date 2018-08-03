#include <stdio.h>
#include <stdlib.h>
#include "stdafx.h"
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <memory.h>

extern int errno;
//Holds RGB values
struct rgb {
	int r, g, b;
};

//Code constants
const int iterations = 5000; //Number of iterations for integration
const int minSteps = 300; //Minimum number of steps before pendulum() breaks
const double MAXFLOAT = 99999999.9;

//Coordinate System parameters
const double minX = -15.0;
const double maxX = 15.0;
const double minY = -15.0;
const double maxY = 15.0;

//Image Parameters
const unsigned imageWidth = 300;
const unsigned imageHeight = 300;

//Physics constants
const double k_f = 0.001; //friction constant
const double k_g = 0.0;  //pendulum constant
const double k_m = 0.3; //magnet force constant
const double m_fHeight = 1.0; //pendulum height above magnets
							  //All force sources positions
							  // value 1: magnet's x coordinate
							  // value 2: magnet's y coordinate
#define ORIGINDIST 5.0 //distance the magnets are from x: 0 y:0
#define NUM_MAGNETS 3

double magnets[NUM_MAGNETS + 1][2] = {
	{ 0,0 }, // <---- Mountpoint location
{ 0,-ORIGINDIST - 15 },
{ -ORIGINDIST - 0.5,0 },
{ ORIGINDIST + 0.5,0 },
};

// Determines which magnet the pendulumn is going to land
// double x: starting x coordinate of pendulum
// double y: starting y coordinate of pendulum
int pendulum(double x, double y) {
	//Closests magnet
	int closest_magnet = -1;

	//Starting values
	double pos[2] = { x,y };
	double vel[2] = { 0,0 }; //starting velocity
	double r[2] = { 0,0 }; //starting position
	double acc0[2] = { 0,0 }; //pendulum acceleration
	double acc1[2] = { 0,0 }; //pendulumn accelertion next step
	double acc2[2] = { 0,0 }; //accel prev step
	double force[2] = { 0,0 }; //x & y forces on pendulum
	double *tmp, *acc_n, *acc_p, *acc, t,dt, closest_dist, dist;

	tmp = malloc(2 * sizeof(double));
	acc_n = acc2;
	acc_p = acc0;
	acc = acc1;

	//Time variables
	t = 0;
	dt = 0.1;

	//Calculate source's forces
	for (int ct = 0; ct<iterations; ++ct) {
		//Update time
		t += dt;
		//Update position
		pos[0] += vel[0] * dt + dt * dt * (2.0 / 3.0 * acc[0] - 1.0 / 6.0 * acc_p[0]);
		pos[1] += vel[1] * dt + dt * dt * (2.0 / 3.0 * acc[1] - 1.0 / 6.0 * acc_p[1]);

		//reset acceleration
		acc_n[0] = 0.0;
		acc_n[1] = 0.0;

		closest_dist = MAXFLOAT;
		//Calculate the magnet and pendulum forces on the pendulumn acceleration
		for (int i = 0; i<NUM_MAGNETS + 1; i++) {
			const double* src = magnets[i];

			r[0] = pos[0] - src[0];
			r[1] = pos[1] - src[1];

			//Calculate force
			//Force of mountpoint on pendulum
			if (i == 0) {
				force[0] = k_g * r[0];
				force[1] = k_g * r[1];
				//magnetic force
			}
			else {
				dist = sqrt((src[0] - pos[0])*(src[0] - pos[0]) + (src[1] - pos[1])*(src[1] - pos[1]) + m_fHeight * m_fHeight);

				if (dist<closest_dist) {
					closest_dist = dist;
					closest_magnet = i;
				}
				force[0] = k_m / (dist*dist*dist)*r[0];
				force[1] = k_m / (dist*dist*dist)*r[1];
			}

			//Update acceleration
			acc_n[0] -= force[0];
			acc_n[1] -= force[1];

			//Break case
			if (ct<minSteps&&fabs(r[0])<0.1&&fabs(r[1])<0.1&&fabs(vel[0])<0.05&&fabs(vel[1])<0.05) {
				break;
			}
		}

		// Friction force on pendulum
		acc_n[0] -= vel[0] * k_f;
		acc_n[1] -= vel[1] * k_f;

		// Update velocity
		vel[0] += dt * (1.0 / 3.0 * acc_n[0] + 5.0 / 6.0 * acc[0] - 1.0 / 6.0 * acc_p[0]);
		vel[1] += dt * (1.0 / 3.0 * acc_n[1] + 5.0 / 6.0 * acc[1] - 1.0 / 6.0 * acc_p[1]);

		//Store values
		tmp = acc_p;
		acc_p = acc;
		acc = acc_n;
		acc_n = tmp;
	}
	return closest_magnet;
}

// Creates a square box around the magnet
// double x: x coordinate of pixel
// double y: y coordinate of pixel
int isNearMagnet(double x, double y) {
	double r[2];
	for (int i = 1; i<NUM_MAGNETS + 1; i++) {
		double*magnet = magnets[i];
		r[0] = magnet[0] - x;
		r[1] = magnet[1] - y;
		if (fabs(r[0])<0.4&&fabs(r[1])<0.4) {
			return 1;
		}
	}
	return 0;
}

// Creates image with pixel values calculated from pendulum function
// Prints out color distribution
void printImage(char* imageName) {
	//Image matrix
	struct rgb *mat = (struct rgb*)malloc(imageHeight * imageWidth * sizeof(struct rgb));
	int result;
	double cx, cy;
	char *location, *exten,*name;

	double bucket[NUM_MAGNETS] = { [NUM_MAGNETS - 1] = 0.0 };
	for (unsigned r = 0; r<imageHeight; r++) {
		 cx = minX + (r*1.0 / imageHeight)*(maxX - minX); //constant real
		for (unsigned i = 0; i<imageWidth; i++) {
			cy = minY + (i * 1.0 / imageWidth) * (maxY - minY);
			//Calculate coordinate result
			result = pendulum(cx, cy);
			//determine color
			struct rgb color;
			color.r = 0;
			color.g = 0;
			color.b = 0;

			if (isNearMagnet(cx, cy)) {//If coordinate is magnet
				color.r = 255;
				color.b = 255;
			}
			else {
				switch (result) {
				case 1: color.r = 255;
					break;
				case 2: color.g = 255;
					break;
				case 3: color.b = 255;
					break;
				case 4:
					color.r = 255;
					color.g = 255;
					break;
				case 5:
					color.g = 255;
					color.b = 255;
					break;
				case 6: color.r = 127;
					break;
				case 7: color.g = 127;
					break;
				case 8: color.b = 127;
					break;
				}
			}
			//add color to bucket
			bucket[result - 1]++;
			//add color to matrix
			//The "-1" avoids a possible buffer overrun
			mat[(i + r * imageWidth) - 1] = color;
		}
	}

	//CREATE IMAGE WITH RGB MATRIX
	location = ".\\gif1\\";
	exten = ".png";

	name = malloc(1 + strlen(location) + strlen(imageName) + strlen(exten));
	if (name != 0) {
		strcpy(name, location);
		strcat(name, imageName);
		strcat(name, exten);
	}
	else {
		printf("Variable name is Null exiting.");
		exit(0);
	}
	FILE * fp = fopen(name, "wb");
	if (fp == NULL) {
		int errnum = errno;
		fprintf(stderr, "Value of errno: %d\n", errno);
		perror("Error printed by perror");
		fprintf(stderr, "Error opening file: %s\n", strerror(errnum));
		printf("File %s does not exist.", name);
	}
	else {
		fprintf(fp, "P3\n");
		fprintf(fp, "%d %d\n", imageWidth, imageHeight);
		fprintf(fp, "%d\n", 255);
	}
	for (unsigned i = 0; i<imageHeight; i++) {
		for (unsigned j = 0; j < imageWidth; j++) {
			fprintf(fp, "%d ", mat[(j + i * imageWidth)-1].r);
			fprintf(fp, "%d ", mat[(j + i * imageWidth)-1].g);
			fprintf(fp, "%d    ", mat[(j + i * imageWidth)-1].b);

			if ((i + j) % 13 == 0) fprintf(fp, "\n");
		}
	}
	fclose(fp);
	//Print bucket distribution
	for (int i = 0; i<NUM_MAGNETS; i++) {
		printf("Magnet %d: %f\n", i, bucket[i]);
	}

}

//Execution method
int main() {
	printf("Begin\n");
	int numPics = 100;
	double yval = (2 * (ORIGINDIST + 15)) / numPics;
	for (int i = 0; i<numPics + 1; i++) {
		printf("Running image %d.png\n", i);
		char array[12];
		sprintf(array, "%d", i);
		printf("Image: %s.pgm\n", array);
		magnets[1][1] = magnets[1][1] + yval;
		printImage(array);
	}
	printf("Done");
	return 0;
}
