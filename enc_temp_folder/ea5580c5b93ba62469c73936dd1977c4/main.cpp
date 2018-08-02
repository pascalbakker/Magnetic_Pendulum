// ConsoleApplication1.cpp : Defines the entry point for the console application.

#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 

// Magnetic Pendulum Execution File
// Created by: Pascal Bakker

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <windows.h>
#include <iostream>
#include <sstream>  // for string streams
// To-do
// CLEAN UP CODE
// PARALLELIZE
// RECORD PROGRAM EXECUTION TIME
// TRACE 1 PIXEL'S PATH
using namespace std;

//Holds RGB values
struct rgb {
	int r, g, b;
};

//Code constants
const int iterations = 5000; //number of iterations for integration
const int minSteps = 300; //Minimum number of steps before pendulum() breaks

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
const double originDist = 5.0; //distance the magnets are from x: 0 y:0
const int numOfMagnets = 3;
double magnets[numOfMagnets + 1][2] = {
	{ 0,0 }, // <---- Mountpoint location
	{ 0,-originDist - 15 },
	{ -originDist - 0.5,0 },
	{ originDist + 0.5,0 },
};


// Function: pendulum
// Determines which magnet the pendulumn is going to land
// Parameters
// double x: starting x coordinate of pendulum
// double y: starting y coordinate of pendulum
int pendulum(double x, double y) {
	//Closests magnet
	int closest_magnet = -1;

	//Starting values
	//Vectors
	double pos[2] = { x,y };
	double vel[2] = { 0,0 }; //starting velocity
	double r[2] = { 0,0 }; //starting position
	double acc0[2] = { 0,0 }; //pendulum acceleration
	double acc1[2] = { 0,0 }; //pendulumn accelertion next step
	double acc2[2] = { 0,0 }; //accel prev step
	double force[2] = { 0,0 }; //x & y forces on pendulum

	//Variables
	double* tmp;
	tmp = (double *)malloc(2 * sizeof(double));

	double* acc_n;
	acc_n = acc2;

	double* acc_p;
	acc_p = acc0;

	double* acc;
	acc = acc1;

	//Time variables
	double t = 0;
	double dt = 0.1;

	//Calculate source's forces
	for (int ct = 0; ct<iterations; ++ct) {
		//Update time
		t += dt;
		//Update position
		pos[0] += vel[0] * dt + dt*dt * (2.0 / 3.0 * acc[0] - 1.0 / 6.0 * acc_p[0]);
		pos[1] += vel[1] * dt + dt*dt * (2.0 / 3.0 * acc[1] - 1.0 / 6.0 * acc_p[1]);

		//reset acceleration
		acc_n[0] = 0.0;
		acc_n[1] = 0.0;

		double closest_dist = 100000;
		//Calculate the magnet and pendulum forces on the pendulumn acceleration
		for (int i = 0; i<numOfMagnets + 1; i++) {
			const double* src = magnets[i];

			r[0] = pos[0] - src[0];
			r[1] = pos[1] - src[1];

			//Calculate force
			//Force of mountpoint on pendulum
			if (i == 0) {
				force[0] = k_g*r[0];
				force[1] = k_g*r[1];
				//magnetic force
			}
			else {
				double dist = sqrt((src[0] - pos[0])*(src[0] - pos[0]) + (src[1] - pos[1])*(src[1] - pos[1]) + m_fHeight*m_fHeight);

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
	//Free

	free(pos);
	free(vel);
	free(acc);
	free(acc0);
	free(acc1);
	free(acc2);
	free(acc_n);
	free(acc_p);
	free(force);
	free(tmp);

	return closest_magnet;
}

// Function: isNearMagnet
// Creates a square box around the magnet
// Parameters
// double x: x coordinate of pixel
// double y: y coordinate of pixel
int isNearMagnet(double x, double y) {
	double r[2];
	for (int i = 1; i<numOfMagnets + 1; i++) {
		double*magnet = magnets[i];
		r[0] = magnet[0] - x;
		r[1] = magnet[1] - y;
		if (fabs(r[0])<0.4&&fabs(r[1])<0.4) {
			return 1;
		}
	}
	return 0;
}

// Function: printImage
// Creates image with pixel values calculated from pendulum function
// Prints out color distribution
// Paramters
// NA
void printImage(char* imageName) {
	//Image matrix
	struct rgb *mat = (struct rgb*)malloc(imageHeight * imageWidth * sizeof(struct rgb));
	int result;

	//Bucket
	//int bucket[numOfMagnets] = { [1 ... numOfMagnets - 1] = 0.0 };

	//CREATE IMAGE MATRIX

	//MPI



	//Referenced Schuster's Mandlebrot slides
	for (unsigned r = 0; r<imageHeight; r++) {
		double cx = minX + (r*1.0 / imageHeight)*(maxX - minX); //constant real
		for (unsigned i = 0; i<imageWidth; i++) {
			double cy = minY + (i * 1.0 / imageWidth) * (maxY - minY);
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
			//add color to matrix
			mat[i + r*imageWidth] = color;
		}
	}

	//CREATE IMAGE WITH RGB MATRIX
	char* location = "C:/Users/pasca/Documents/PendTest/";
	char* exten = ".pgm";
	char* name = (char*)malloc(1 + strlen(location) + strlen(imageName) + strlen(exten));
	strcpy(name, location);
	strcat(name, imageName);
	strcat(name, exten);


	printf("File location: ");
	for (size_t i = 0; i < strlen(name); i++) {
		printf("%c", name[i]);
	}
	printf("\n");

	FILE * fp = fopen(name, "wb");
	fprintf_s(fp, "P3\n");
	fprintf_s(fp, "%d %d\n", imageWidth, imageHeight);
	fprintf_s(fp, "%d\n", 255);
	for (unsigned i = 0; i<imageHeight; i++) {
		for (unsigned j = 0; j < imageWidth; j++) {
			fprintf_s(fp, "%d ", mat[j + i * imageWidth].r);
			fprintf_s(fp, "%d ", mat[j + i * imageWidth].g);
			fprintf_s(fp, "%d    ", mat[j + i * imageWidth].b);

			if ((i + j) % 13 == 0) fprintf(fp, "\n");
		}
	}
	fclose(fp);
}

//Execution method
int main() {
	printf("Begin\n");
	int numPics = 100;
	double yval = (2 * (originDist + 15)) / numPics;
	for (int i = 0; i<numPics + 1; i++) {
		printf("\nRunning image %d.png\n", i);
		string test = std::to_string(i);
		char tab2[1024];
		strcpy(tab2, test.c_str());
		printImage(tab2);
	}
	printf("Done");
	return 0;
}
