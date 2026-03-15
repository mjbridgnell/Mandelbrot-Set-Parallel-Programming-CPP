// Maxwell Bridgnell
// CSCI 551
// Final Parallel Program
// Parallel MPI Calculation of the Mandelbrot Set

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/* Pre-set maximum number of iterations to calculate in the Mandelbrot equation. Change width and height
to increase resolution of the set. Keep the ratio to make sure the set is not cut off in the window. */
#define WIDTH 800
#define HEIGHT 600
#define MAX_ITER 255

/* The write_pgm function takes the int array holding the number of iterations it took each pixel to ecape
the set and uses each pixel's number of iterations to represent a value of grey in a .pgm file. Pixels
that reached the maximum iteration of 255 without escaping the set are represented by white. The darker a
pixel is, the less iterations it took to escape the set. */
void write_pgm(const char *filename, int *image)
{
    // Open the file for writing
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }

    // Iterate through each pixel
    fprintf(file, "P2\n%d %d\n255\n", WIDTH, HEIGHT); // Write .pgm file header
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            fprintf(file, "%d ", image[y * WIDTH + x]); // Write pixel value
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
    // Time variables
    double start, end, elapsed;

    // Initialize MPI
    int comm_sz;
    int my_rank;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // Get the start time of rank 0
    if (my_rank == 0)
    {
        start = MPI_Wtime();
    }

    // Split window into equal rows for each process
    int local_HEIGHT = HEIGHT / comm_sz;
    int local_WIDTH = WIDTH;

    // Set start and end heights for each process
    int local_HEIGHT_start = my_rank * local_HEIGHT;
    int local_HEIGHT_end = local_HEIGHT_start + local_HEIGHT;

    // Allocate memory for the each process's Mandelbrot set
    int *mandelbrot_set_local = (int *)malloc(local_WIDTH * local_HEIGHT * sizeof(int));
    if (!mandelbrot_set_local)
    {
        perror("Memory allocation failed");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // Initialize mins and maxes, the set resides within this boundary
    double real_min = -2.5, real_max = 1;
    double imag_min = -1, imag_max = 1;

    /* Iterate through each pixel in each process's window and calculate the number of iterations each
    pixel's corresponding complex number took to escape the set. */
    for (int y = local_HEIGHT_start; y < local_HEIGHT_end; y++)
    {
        for (int x = 0; x < local_WIDTH; x++)
        {
            double real = real_min + x * (real_max - real_min) / (double)WIDTH;
            double imag = imag_min + y * (imag_max - imag_min) / (double)HEIGHT;

            mandelbrot_set_local[(y - local_HEIGHT_start) * local_WIDTH + x] = mandelbrot(real, imag);
        }
    }

    int *mandelbrot_set = NULL;
    if (my_rank == 0)
    {
        // Allocate memory for the complete Mandelbrot set
        mandelbrot_set = (int *)malloc(WIDTH * HEIGHT * sizeof(int));
        if (!mandelbrot_set)
        {
            perror("Memory allocation failed");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    // Gather each process's set and input it into the complete set
    MPI_Gather(mandelbrot_set_local, local_WIDTH * local_HEIGHT, MPI_INT,
               mandelbrot_set, local_WIDTH * local_HEIGHT, MPI_INT, 0, MPI_COMM_WORLD);

    // MPI_Barrier to make sure each process is complete before taking the final computation time
    MPI_Barrier(MPI_COMM_WORLD);
    if (my_rank == 0)
    {
        // Calculate and output computation time
        end = MPI_Wtime();
        elapsed = end - start;
        printf("Rank %d: Elapsed time = %f seconds\n", my_rank, elapsed);
    }

    MPI_Finalize();

    if (my_rank == 0)
    {
        // Write Set the .pgm file
        write_pgm("mandelbrot_mpi.pgm", mandelbrot_set);
        free(mandelbrot_set);
    }

    free(mandelbrot_set_local);
    return 0;
}
