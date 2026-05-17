#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "mandel.h"
#include "timer.h"

#define MAX_ITER 255 // Pre-set maximum number of iterations to calculate in the Mandelbrot equation.

// Initialize mins and maxes, the set resides within this boundary
const double real_min = -2.5, real_max = 1;
const double imag_min = -1, imag_max = 1;

/* The write_pgm function takes the int array holding the number of iterations it took each pixel to ecape
the set and uses each pixel's number of iterations to represent a value of grey in a .pgm file. Pixels
that reached the maximum iteration of 255 without escaping the set are represented by white. The darker a
pixel is, the less iterations it took to escape the set. */
void write_pgm(const char *filename, const int *image, int width, int height)
{
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }

    // Iterate through each pixel
    fprintf(file, "P2\n%d %d\n255\n", width, height); // Write .pgm file header
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            fprintf(file, "%d ", image[y * width + x]); // Write pixel value
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

/* The mandelbrot function takes a complex number and inputs it into the Mandelbrot set equation. The function
returns the number of iterations it took for a number to escape the set. When the square of the real part plus
the square of the imaginary part of a complex number is greater than 4, it is assumed the number will blow up
to infinity and not part of the set. The less iterations a number took to escape the set, the further it is
from the set. If a number reaches the pre-set max number of iterations, 255, it is considered to be
in the set. */
int mandelbrot(double real, double imag)
{
    // z_real and z_imag represent Z n = 0, they start as 0 for every complex number
    double z_real = 0, z_imag = 0;
    int iter = 0;

    while (z_real * z_real + z_imag * z_imag <= 4 && iter < MAX_ITER) // Escape condition
    {
        double temp = z_real * z_real - z_imag * z_imag + real; // Update real part of z
        z_imag = 2 * z_real * z_imag + imag;                    // Update imaginary part of z
        z_real = temp;
        iter++;
    }

    return iter; // Return number of iterations taken to escape
}

/* The main function takes input for the window size from the user, sends each pixel's complex number to the
Mandelbrot equation, times the set's calculation, and sends the set to the write_pgm function to be written
as a .pgm file. */
void mandel::mandel(int width, int height)
{
    // Allocate memory for the Mandelbrot set
    int *mandelbrot_set = new int[width * height];
    if (!mandelbrot_set)
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    Timer t;

    /* Iterate through each pixel in the window and calculate the number of iterations each
    pixel's corresponding complex number took to escape the set. */
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // Calculate each pixel's complex number
            double real = real_min + x * (real_max - real_min) / (double)width;  // Real part
            double imag = imag_min + y * (imag_max - imag_min) / (double)height; // Imaginary part
            mandelbrot_set[y * width + x] = mandelbrot(real, imag);
        }
    }

    double time = t.elapsed();

    printf("Mandelbrot Calculation Time: %.9f seconds\n", time);

    write_pgm("mandelbrot.pgm", mandelbrot_set, width, height);
    delete[] mandelbrot_set;
}

void mandel::mandel_omp(int width, int height, int num_threads)
{
    int *mandelbrot_set = new int[height * width];
    if (!mandelbrot_set)
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    Timer t;

#pragma omp parallel for num_threads(num_threads)
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            double real = real_min + x * (real_max - real_min) / (double)width;
            double imag = imag_min + y * (imag_max - imag_min) / (double)height;
            mandelbrot_set[y * width + x] = mandelbrot(real, imag);
        }
    }

    double time = t.elapsed();

    printf("Mandelbrot Calculation Time: %.9f seconds\n", time);

    write_pgm("mandelbrot_openmp.pgm", mandelbrot_set, width, height);
    delete[] mandelbrot_set;
}