#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "timer.h"

// mpirun -np 4 ./mandel_mpi

/* Pre-set maximum number of iterations to calculate in the Mandelbrot equation. Change width and height
to increase resolution of the set. Keep the ratio to make sure the set is not cut off in the window. */
#define WIDTH 800
#define HEIGHT 600
#define MAX_ITER 255

void write_pgm(const char *filename, const int *image)
{
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "P2\n%d %d\n255\n", WIDTH, HEIGHT);
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            fprintf(file, "%d ", image[y * WIDTH + x]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

int mandelbrot(double real, double imag)
{
    double z_real = 0, z_imag = 0;
    int iter = 0;

    while (z_real * z_real + z_imag * z_imag <= 4 && iter < MAX_ITER)
    {
        double temp = z_real * z_real - z_imag * z_imag + real;
        z_imag = 2 * z_real * z_imag + imag;
        z_real = temp;
        iter++;
    }

    return iter;
}

int main()
{
    // Initialize MPI
    int comm_sz;
    int my_rank;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    Timer t;

    // Split window into equal rows for each process
    const int local_HEIGHT = HEIGHT / comm_sz;
    const int local_WIDTH = WIDTH;

    // Set start and end heights for each process
    const int local_HEIGHT_start = my_rank * local_HEIGHT;
    const int local_HEIGHT_end = local_HEIGHT_start + local_HEIGHT;

    // Allocate memory for the each process's Mandelbrot set
    int *mandelbrot_set_local = new int[local_WIDTH * local_HEIGHT];
    if (!mandelbrot_set_local)
    {
        perror("Memory allocation failed");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // Initialize mins and maxes, the set resides within this boundary
    const double real_min = -2.5, real_max = 1;
    const double imag_min = -1, imag_max = 1;

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
        mandelbrot_set = new int[WIDTH * HEIGHT];
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
        double time = t.elapsed();
        printf("Rank %d: Elapsed time = %f seconds\n", my_rank, time);
    }

    MPI_Finalize();

    if (my_rank == 0)
    {
        write_pgm("mandelbrot_mpi.pgm", mandelbrot_set);
        delete[] mandelbrot_set;
    }

    delete[] mandelbrot_set_local;
    return 0;
}
