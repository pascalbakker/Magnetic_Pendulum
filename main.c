/*
 * ----------------------------------------------------------------------------
 * File: pendulum_simulation.c
 * Description: Simulation of a pendulum with magnet interaction, including
 *              friction and gravity forces. Used to show example of chaos
 * theory.
 *
 * Author: Pascal Bakker
 * Date Created: March 30, 2025
 * Last Modified: March 30, 2025
 * Version: 1.0
 *
 * License:  MIT License
 * ----------------------------------------------------------------------------
 *  TODO
 *  - parallelize program
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_PATH "test.pgm"
#define NUM_OF_MAGNETS 4
#define MAXFLOAT 10000.0
#define ITERATIONS 5000 // number of iterations for integration
#define minSteps 300    // Minimum number of steps before pendulum() breaks
// Coordinate System parameters
#define minX -15.0
#define maxX 15.0
#define minY -15.0
#define maxY 15.0
// Image Parameters
#define imageWidth 300
#define imageHeight 300
// Physics constants
#define k_f 0.001      // friction constant
#define k_g 0.0        // pendulum constant
#define k_m 0.3        // magnet force constant
#define m_fHeight 1.0  // pendulum height above magnets
#define originDist 5.0 // distance the magnets are from x: 0 y:0
#define numOfMagnets 4

// Holds RGB values
struct rgb
{
  int r, g, b;
};

// All force sources positions
//  value 1: magnet's x coordinate
//  value 2: magnet's y coordinate
double magnets[NUM_OF_MAGNETS + 1][2] = {
  { 0, 0 }, // <---- Mountpoint location
  { originDist, originDist },
  { -originDist, originDist },
  { originDist, -originDist },
  { -originDist, -originDist },
};

// Determines which magnet the pendulum is going to land
// Parameters
// double x: starting x coordinate of pendulum
// double y: starting y coordinate of pendulum
int
pendulum (double x, double y)
{
  // Closests magnet
  int closest_magnet = -1;

  // Starting values
  // Vectors
  double pos[2] = { x, y };
  double vel[2] = { 0, 0 };   // starting velocity
  double r[2] = { 0, 0 };     // starting position
  double acc0[2] = { 0, 0 };  // pendulum acceleration
  double acc1[2] = { 0, 0 };  // pendulumn accelertion next step
  double acc2[2] = { 0, 0 };  // accel prev step
  double force[2] = { 0, 0 }; // x & y forces on pendulum

  // Accelertion variables
  double *acc_n = acc2;
  double *acc_p = acc0;
  double *acc = acc1;

  // Time variables
  double t = 0;
  double dt = 0.1;

  // Calculate source's forces
  for (int ct = 0; ct < ITERATIONS; ++ct)
    {
      // Update time
      t += dt;
      // Update position
      pos[0] += vel[0] * dt
                + dt * dt * (2.0 / 3.0 * acc[0] - 1.0 / 6.0 * acc_p[0]);
      pos[1] += vel[1] * dt
                + dt * dt * (2.0 / 3.0 * acc[1] - 1.0 / 6.0 * acc_p[1]);

      // reset acceleration
      acc_n[0] = 0.0;
      acc_n[1] = 0.0;

      double closest_dist = MAXFLOAT;
      // Calculate the magnet and pendulum forces on the pendulumn acceleration
      for (int i = 0; i < numOfMagnets + 1; i++)
        {
          const double *src = magnets[i];

          r[0] = pos[0] - src[0];
          r[1] = pos[1] - src[1];

          // Calculate force
          // Force of mountpoint on pendulum
          if (i == 0)
            {
              force[0] = k_g * r[0];
              force[1] = k_g * r[1];
              // magnetic force
            }
          else
            {
              double dist = sqrt ((src[0] - pos[0]) * (src[0] - pos[0])
                                  + (src[1] - pos[1]) * (src[1] - pos[1])
                                  + m_fHeight * m_fHeight);

              if (dist < closest_dist)
                {
                  closest_dist = dist;
                  closest_magnet = i;
                }
              force[0] = k_m / (dist * dist * dist) * r[0];
              force[1] = k_m / (dist * dist * dist) * r[1];
            }

          // Update acceleration
          acc_n[0] -= force[0];
          acc_n[1] -= force[1];

          // Break case
          if (ct < minSteps && fabs (r[0]) < 0.1 && fabs (r[1]) < 0.1
              && fabs (vel[0]) < 0.05 && fabs (vel[1]) < 0.05)
            {
              break;
            }
        }

      // Friction force on pendulum
      acc_n[0] -= vel[0] * k_f;
      acc_n[1] -= vel[1] * k_f;

      // Update velocity
      vel[0] += dt
                * (1.0 / 3.0 * acc_n[0] + 5.0 / 6.0 * acc[0]
                   - 1.0 / 6.0 * acc_p[0]);
      vel[1] += dt
                * (1.0 / 3.0 * acc_n[1] + 5.0 / 6.0 * acc[1]
                   - 1.0 / 6.0 * acc_p[1]);

      // Store values
      double *tmp = acc_p;
      tmp = acc_p;
      acc_p = acc;
      acc = acc_n;
      acc_n = tmp;
    }
  return closest_magnet;
}

// Creates a square box around the magnet
int
isNearMagnet (double x, double y)
{
  double r[2];
  for (int i = 1; i < NUM_OF_MAGNETS + 1; i++)
    {
      double *magnet = magnets[i];
      r[0] = magnet[0] - x;
      r[1] = magnet[1] - y;
      if (fabs (r[0]) < 0.4 && fabs (r[1]) < 0.4)
        {
          return 1;
        }
    }
  return 0;
}

// Creates image with pixel values calculated from pendulum function
void
printImage ()
{
  struct rgb *mat
      = (struct rgb *)malloc (imageHeight * imageWidth * sizeof (struct rgb));
  if (!mat)
    {
      fprintf (stderr, "Memory allocation failed for image matrix\n");
      exit (1);
    }

  int bucket[NUM_OF_MAGNETS] = { 0 };

  // CREATE IMAGE MATRIX
  for (unsigned r = 0; r < imageHeight; r++)
    {
      double cx = minX + (r * 1.0 / imageHeight) * (maxX - minX);
      for (unsigned i = 0; i < imageWidth; i++)
        {
          double cy = minY + (i * 1.0 / imageWidth) * (maxY - minY);
          int result = pendulum (cx, cy);
          struct rgb color = { 0, 0, 0 };

          if (isNearMagnet (cx, cy))
            {
              color.r = 255;
              color.b = 255;
            }
          else
            {
              switch (result)
                {
                case 1:
                  color.r = 255;
                  break;
                case 2:
                  color.g = 255;
                  break;
                case 3:
                  color.b = 255;
                  break;
                case 4:
                  color.r = 255;
                  color.g = 255;
                  break;
                case 5:
                  color.g = 255;
                  color.b = 255;
                  break;
                case 6:
                  color.r = 127;
                  break;
                case 7:
                  color.g = 127;
                  break;
                case 8:
                  color.b = 127;
                  break;
                }
            }

          bucket[result - 1]++;
          mat[i + r * imageWidth] = color;
        }
    }

  // CREATE IMAGE WITH RGB MATRIX
  const char *home = getenv ("HOME");
  char filePath[256];
  snprintf (filePath, sizeof (filePath), "%s%s", home, FILE_PATH);
  FILE *fp = fopen (filePath, "wb");
  if (!fp)
    {
      fprintf (stderr, "Failed to open output file\n");
      exit (1);
    }

  fprintf (fp, "P3\n");
  fprintf (fp, "%d %d\n", imageWidth, imageHeight);
  fprintf (fp, "%d\n", 255);
  for (unsigned i = 0; i < imageHeight; i++)
    {
      for (unsigned j = 0; j < imageWidth; j++)
        {
          fprintf (fp, "%d %d %d ", mat[j + i * imageWidth].r,
                   mat[j + i * imageWidth].g, mat[j + i * imageWidth].b);
        }
      fprintf (fp, "\n");
    }

  fclose (fp);
  free (mat);
  mat = NULL;
}

int
main ()
{
  printImage ();
  return 0;
}
