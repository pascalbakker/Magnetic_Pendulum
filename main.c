#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

struct rgb{
    int r,g,b;
};

//Code constants
const int iterations = 5000; //number of iterations for integration
const double originDist = 5.0; //distance the magnets are from x: 0 y:0
const int minSteps = 300; //Minimum number of steps
const double minX = -15.0;
const double maxX = 15.0;
const double minY = -15.0;
const double maxY = 15.0;

const unsigned imageWidth = 300;
const unsigned imageHeight = 300;

//Physics constants
const double k_f = 0.001;
const double k_g = 0.0;
const double k_m = 0.3;

const double m_fHeight = 1.0;
const int numOfMagnets = 4;
double magnets[numOfMagnets+1][2] = {
        {0,0},
        {originDist,originDist},
        {-originDist,originDist},
        {originDist,-originDist},
        {-originDist,-originDist},
};

//Creates a magnet visually GUI
int isNearMagnet(double x, double y){
    double r[2];
    for(int i=1;i<numOfMagnets+1;i++){
        double*magnet = magnets[i];
        r[0] = magnet[0]-x;
        r[1] = magnet[1]-y;
        if(fabs(r[0])<0.4&&fabs(r[1])<0.4){
            return 1;
        }
    }
    return 0;
}


//Determines which magnet the pendulumn is going to land
int pendulum(double x, double y){
    //Starting values
    //Vectors
    double pos[2] = {x,y};
    double vel[2] = {0,0}; //starting velocity
    double r[2] = {0,0}; //starting position
    double acc0[2] = {0,0}; //pendulum acceleration
    double acc1[2] = {0,0}; //pendulumn accelertion next step
    double acc2[2] = {0,0}; //accel prev step
    double force[2] = {0,0};

    //Variables
    double* tmp;
    tmp = malloc(2* sizeof(double));

    double* acc_n;
    acc_n = malloc(2* sizeof(double));
    acc_n = acc2;

    double* acc_p;
    acc_p = malloc(2* sizeof(double));
    acc_p = acc0;

    double* acc;
    acc = malloc(2 * sizeof(double));
    acc = acc1;

    double t = 0;
    double dt = 0.1;
    double len = 0;
    int closest_src = -1;

    for(int ct = 0;ct<iterations;++ct){
        //printf("Position: %f %f\n",pos[0],pos[1]);
        //printf("Velocity: %f %f\n",vel[0],vel[1]);

        assert(acc_p);
        assert(acc);
        assert(acc_n);

        t+=dt;

        pos[0] += vel[0] * dt + dt*dt * (2.0 / 3.0 * acc[0] - 1.0 / 6.0 * acc_p[0]);
        pos[1] += vel[1] * dt + dt*dt * (2.0 / 3.0 * acc[1] - 1.0 / 6.0 * acc_p[1]);

        //reset acceleration
        acc_n[0] = 0.0;
        acc_n[1] = 0.0;

        double closest_dist = MAXFLOAT;
        //Calculate the magnet and pendulum forces on the pendulumn acceleration
        for(int i=0;i<numOfMagnets+1;i++){
            const double* src = magnets[i];

            r[0] = pos[0] - src[0];
            r[1] = pos[1] - src[1];

            //Calculate force
            //MOUNTPOINT
            if(i==0){
                force[0] = k_g*r[0];
                force[1] = k_g*r[1];
            //MAGNET
            }else{
                double dist = sqrt((src[0]-pos[0])*(src[0]-pos[0])+(src[1]-pos[1])*(src[1]-pos[1])+m_fHeight*m_fHeight);

                if(dist<closest_dist){
                    closest_dist = dist;
                    closest_src = i;
                }

                force[0] = k_m/(dist*dist*dist)*r[0];
                force[1] = k_m/(dist*dist*dist)*r[1];
            }

            acc_n[0] -= force[0];
            acc_n[1] -= force[1];

            //Break case
            if(ct<minSteps&&fabs(r[0])<0.1&&fabs(r[1])<0.1&&fabs(vel[0])<0.05&&fabs(vel[1])<0.05){
                break;
            }

        }

        acc_n[0] -= vel[0]*k_f;
        acc_n[1] -= vel[1]*k_f;


        vel[0] += dt * (1.0 / 3.0 * acc_n[0] + 5.0 / 6.0 * acc[0] - 1.0 / 6.0 * acc_p[0]);
        vel[1] += dt * (1.0 / 3.0 * acc_n[1] + 5.0 / 6.0 * acc[1] - 1.0 / 6.0 * acc_p[1]);

        //--------------------------------------------------------------
        // 5.) flip the acc buffer
        tmp = acc_p;
        acc_p = acc;
        acc = acc_n;
        acc_n = tmp;
    }

    return closest_src;
}

// Prints image
//Creates rgb image
//Prints out color distribution
void printImage(){
    //Image matrix
    struct rgb *mat = (struct rgb*)malloc(imageHeight * imageWidth* sizeof(struct rgb));
    int result;

    //Bucket
    int bucket[numOfMagnets] = {[1 ... numOfMagnets-1] = 0.0};

    //CREATE IMAGE MATRIX
    //Referenced Schuster's Mandlebrot slides
    for(unsigned r=0;r<imageHeight;r++){
        double cx = minX+(r*1.0/imageHeight)*(maxX-minX); //constant real
        for(unsigned i=0;i<imageWidth;i++) {
            double cy = minY + (i * 1.0 / imageWidth) * (maxY - minY);
            result = pendulum(cx, cy);
            //determine color
            struct rgb color;
            if (isNearMagnet(cx,cy)) {
                color.r= 255;
                color.g = 0;
                color.b = 255;
            }else if(result==1){
                color.r= 255;
                color.g = 0;
                color.b = 0;
            }else if(result==2){
                color.r = 0;
                color.g = 255;
                color.b = 0;
            }else if(result==3){
                color.r = 0;
                color.g = 0;
                color.b = 255;
            }else if(result==4){
                color.r = 255;
                color.g = 255;
                color.b = 0;
            }else if(result==5){
                color.r = 0;
                color.g = 255;
                color.b = 255;
            } else{
                color.r = 0;
                color.g = 0;
                color.b = 0;
            }
            //add color to bucket
            bucket[result-1]++;
            //add color to matrix
            mat[i+r*imageWidth] = color;
        }
    }

    //CREATE IMAGE WITH RGB MATRIX
    FILE * fp = fopen ("/Users/pascal/CLionProjects/ParallelMagnet/test.pgm","wb");
    fprintf(fp,"P3\n");
    fprintf(fp,"%d %d\n",imageWidth,imageHeight);
    fprintf(fp,"%d\n",255);
    for(unsigned i = 0;i<imageHeight; i++) {
        for (unsigned j = 0; j < imageWidth; j++) {
            fprintf(fp, "%d ", mat[j + i * imageWidth].r);
            fprintf(fp, "%d ", mat[j + i * imageWidth].g);
            fprintf(fp, "%d    ", mat[j + i * imageWidth].b);

            if((i+j)%13==0) fprintf(fp,"\n");
        }
    }


    //Print bucket distribution
    for(int i=0;i<numOfMagnets;i++){
        printf("Magnet %d: %d\n",i,bucket[i]);
    }
}



int main() {
    printImage();




    return 0;


}