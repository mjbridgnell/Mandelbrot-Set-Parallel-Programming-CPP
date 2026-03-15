// Maxwell Bridgnell
// CSCI 551
// Final Parallel Program
// Parallel OpenMP Calculation of Mandelbrot Set

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

// Pre-set maximum number of iterations to calculate in the Mandelbrot equation.
#define MAX_ITER 255
#define NUM_THREADS 8

/* The write_pgm function takes the int array holding the number of iterations it took each pixel to ecape
the set and uses each pixel's number of iterations to represent a value of grey in a .pgm file. Pixels
that reached the maximum iteration of 255 without escaping the set are represented by white. The darker a
pixel is, the less iterations it took to escape the set. */
void write_pgm(const char *filename, int *image, int width, int height)
{
    // Open the file for writing
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
        fprintf(file, "\n"); // New line after each row
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
    // z_real and z_imag represent Z n = 0, they start of as 0 for every complex number
    double z_real = 0, z_imag = 0;
    int iter = 0; // initialize iteration count

    while (z_real * z_real + z_imag * z_imag <= 4 && iter < MAX_ITER) // Escape condition
    {
        double temp = z_real * z_real - z_imag * z_imag + real; // Update real part of z
        z_imag = 2 * z_real * z_imag + imag;                    // Update imaginary part of z
        z_real = temp;
        iter++; // Increment iteration count
    }

    return iter; // Return number of iterations taken to escape
}

/* The main function takes input for the window size from the user, sends each pixel's complex number to the
Mandelbrot equation, times the set's calculation, and sends the set to the write_pgm function to be written
as a .pgm file. */
int main()
{
    int width, height; // Width and height of the image
    struct timespec start, end;
    double diff;

    // Take in input for width and height
    printf("Enter the width of the image, recommend 800, increase to increase image resolution: ");
    scanf("%d", &width);
    printf("Enter the height of the image, recommend 600, increase to increase image resolution: ");
    scanf("%d", &height);

    // Allocate memory for the Mandelbrot set
    int *mandelbrot_set = malloc(width * height * sizeof(int));
    if (!mandelbrot_set)
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize mins and maxes, the set resides within this boundary
    double real_min = -2.5, real_max = 1;
    double imag_min = -1, imag_max = 1;

    if (clock_gettime(CLOCK_MONOTONIC_RAW, &start) != 0)
    {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }

    /* Iterate through each pixel in the window and calculate the number of iterations each
    pixel's corresponding complex number took to escape the set. */
#pragma omp parallel for num_threads(NUM_THREADS)
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // Calculate each pixel's complex number
            double real = real_min + x * (real_max - real_min) / (double)width;  // Real part
            double imag = imag_min + y * (imag_max - imag_min) / (double)height; // Imaginary part
            // Send complex number to Mandelbrot equation
            mandelbrot_set[y * width + x] = mandelbrot(real, imag);
        }
    }

    if (clock_gettime(CLOCK_MONOTONIC_RAW, &end) != 0)
    {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }

    // Calculation time
    diff = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1.0e9;
    printf("Mandelbrot Calculation Time: %.9f seconds\n", diff);

    // Write Set the .pgm file
    write_pgm("mandelbrot.pgm", mandelbrot_set, width, height);
    free(mandelbrot_set);

    return 0;
}
