#include <stdio.h>
#include <stdlib.h>
#include "stdafx.h"
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

//Holds RGB values
struct rgb {
	int r, g, b;
};
#define ORIGINDIST 5.0 //distance the magnets are from x: 0 y:0
#define NUM_MAGNETS 3
//Code constants
const int iterations = 5000; //number of iterations for integration
const int minSteps = 300; //Minimum number of steps before pendulum() breaks

//Image Parameters
const unsigned imageWidth = 300;
const unsigned imageHeight = 300;



// Determines which magnet the pendulumn is going to land
__device__ int pendulum(double x, double y, double dev_magnets[NUM_MAGNETS + 1][2]) {
	const float MAXFLOAT = 99999999.9;
	int closest_magnet = -1, ct;
	double *tmp, *acc_p, *acc, *acc_n, t, dt, closest_dist, dist;
	const double k_f = 0.001; //friction constant
	const double k_g = 0.0;  //pendulum constant
	const double k_m = 0.3; //magnet force constant
	const double m_fHeight = 1.0; //pendulum height above magnets
								  //All force sources positions
								  // value 1: magnet's x coordinate
								  // value 2: magnet's y coordinate
								  //Coordinate System parameters

	//Starting Vectors
	double pos[2] = { x,y };
	double vel[2] = { 0,0 }; //starting velocity
	double r[2] = { 0,0 }; //starting position
	double acc0[2] = { 0,0 }; //pendulum acceleration
	double acc1[2] = { 0,0 }; //pendulumn accelertion next step
	double acc2[2] = { 0,0 }; //accel prev step
	double force[2] = { 0,0 }; //x & y forces on pendulum

							   
	//tmp = malloc(2 * sizeof(double));

	acc_n = acc2;

	acc_p = acc0;

	acc = acc1;

	//Time variables
	t = 0;
	dt = 0.1;

	//Calculate source's forces
	for (ct = 0; ct < iterations; ++ct) {
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
		for (int i = 0; i < NUM_MAGNETS + 1; i++) {
			double *src = dev_magnets[i];

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

				if (dist < closest_dist) {
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
			if (ct < minSteps&&fabs(r[0]) < 0.1&&fabs(r[1]) < 0.1&&fabs(vel[0]) < 0.05&&fabs(vel[1]) < 0.05) {
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
__device__ int isNearMagnet(double x, double y,double dev_magnets[NUM_MAGNETS + 1][2]) {
	double r[2], *magnet;
	int i;
	for (i = 1; i < NUM_MAGNETS + 1; i++) {
		magnet = dev_magnets[i];
		r[0] = magnet[0] - x;
		r[1] = magnet[1] - y;
		if (fabs(r[0]) < 0.4&&fabs(r[1]) < 0.4) {
			return 1;
		}
	}
	return 0;
}


// Creates image with pixel values calculated from pendulum function and prints out color distribution
void printImage(char* imageName, struct rgb *mat, int *bucket[NUM_MAGNETS]) {
	char *location, *exten;
	//CREATE IMAGE WITH RGB MATRIX
	location = "C:/Users/rcall/source/repos/CUDA_Final/ConsoleApplication1/gif1/";
	exten = ".png";

	char *name = (char*)malloc(1 + strlen(location) + strlen(imageName) + strlen(exten));

	if (name != 0) {
		strcpy(name, location);
		strncat(name, imageName,1);
		strncat(name, exten,4);
	}
	else {
		printf("Variable name is Null exiting.");
		exit(0);
	}
	FILE * fp = fopen(name, "wb");
	if (fp == NULL) {
		printf("File %s does not exist.", name);
		exit(0);
	}
	else {
		fprintf(fp, "P3\n");
		fprintf(fp, "%d %d\n", imageWidth, imageHeight);
		fprintf(fp, "%d\n", 255);
	}
	for (unsigned i = 0; i < imageHeight; i++) {
		for (unsigned j = 0; j < imageWidth; j++) {
			fprintf(fp, "%d ", mat[(j + i * imageWidth)].r);
			fprintf(fp, "%d ", mat[(j + i * imageWidth)].g);
			fprintf(fp, "%d    ", mat[(j + i * imageWidth)].b);

			if ((i + j) % 13 == 0) fprintf(fp, "\n");
		}
	}
	fclose(fp);
	//Print bucket distribution
	for (int i = 0; i < NUM_MAGNETS; i++) {
		printf("Magnet %d: %f\n", i, bucket[i]);
	}

}

__global__ void Calculate(struct rgb *dev_mat, int *dev_bucket,double dev_magnets[NUM_MAGNETS + 1][2]) {
	const double minX = -15.0;
	const double maxX = 15.0;
	const double minY = -15.0;
	const double maxY = 15.0;

	//dev_mat = (struct rgb*)malloc(imageHeight * imageWidth * sizeof(struct rgb));
	//dev_bucket[NUM_MAGNETS] = { dev_bucket[NUM_MAGNETS] = 0 };

	//Image matrix
	int result;
	double cx, cy;
	unsigned r, i;
	//Bucket

	//Referenced Schuster's Mandlebrot slides
	for (r = 0; r < imageHeight; r++) {
		cx = minX + (r*1.0 / imageHeight)*(maxX - minX); //constant real
		for (i = 0; i < imageWidth; i++) {
			cy = minY + (i * 1.0 / imageWidth) * (maxY - minY);
			//Calculate coordinate result
			result = pendulum(cx, cy,dev_magnets);
			//determine color
			struct rgb color;
			color.r = 0;
			color.g = 0;
			color.b = 0;

			if (isNearMagnet(cx, cy,dev_magnets)) {//If coordinate is magnet
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
			dev_bucket[result - 1]++;
			//add color to matrix
			dev_mat[(i + r * imageWidth)] = color;
		}
	}

}

//Execution method
int main() {
	printf("Begin\n");
	int numPics = 100;
	int yval = (2 * (ORIGINDIST + 15)) / numPics;
	double bytes = imageHeight * imageWidth * sizeof(struct rgb);

	struct rgb *dev_mat;
	int  *dev_bucket;
	double  *dev_magnets;

	struct rgb *mat = (struct rgb*)malloc(bytes);
	int *bucket[NUM_MAGNETS] = { bucket[NUM_MAGNETS - 1] = 0 };
	double magnets[NUM_MAGNETS + 1][2] = {{0,0},{0,-ORIGINDIST - 15},{-ORIGINDIST - 0.5,0},{ORIGINDIST + 0.5,0}};

	cudaMalloc((void**)&dev_mat, bytes);
	cudaMalloc((void**)&dev_bucket, bytes);
	cudaMalloc((void**)&dev_magnets, bytes);

	cudaMemcpy(dev_mat, &mat, bytes, cudaMemcpyHostToDevice);
	printf("Device Mat Variable Copying:\t%s\n", cudaGetErrorString(cudaGetLastError()));
	cudaMemcpy(dev_bucket, &bucket, bytes, cudaMemcpyHostToDevice);
	printf("Device Bucket Variable Copying:\t%s\n", cudaGetErrorString(cudaGetLastError()));
	cudaMemcpy(dev_magnets, &magnets, bytes, cudaMemcpyHostToDevice);
	printf("Device Magnet Variable Copying:\t%s\n", cudaGetErrorString(cudaGetLastError()));

	for (int i = 0; i < numPics + 1; i++) {
		printf("Running image %d.png\n", i);
		char array[12];
		sprintf(array, "%d", i);
		printf("Image: %s.pgm\n", array);
		magnets[1][1] = magnets[1][1] + yval;
		//Calculate<<<1,1>>>(dev_mat,dev_bucket,dev_magnets);
		cudaDeviceSynchronize();
		cudaError_t error = cudaGetLastError();
		if (error != cudaSuccess)
		{
			fprintf(stderr, "ERROR: %s\n", cudaGetErrorString(error));
			exit(-1);
		}
		cudaMemcpy(dev_magnets, magnets, bytes, cudaMemcpyDeviceToHost);
		cudaMemcpy(dev_bucket, bucket, bytes, cudaMemcpyDeviceToHost);
		cudaMemcpy(mat, dev_mat, bytes, cudaMemcpyDeviceToHost);
		printf("Test Magnet %d: %f\n", i, magnets);
		printImage(array,mat,bucket);
	}
	printf("Done");
	return 0;
}