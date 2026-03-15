CC = gcc
CFLAGS = -Wall -Wextra

PRODUCT= mandel mandel_omp mandel_mpi
CFILES= mandel.c mandel_openmp.c mandel_mpi.c

SRCS= ${CFILES}
OBJS= ${CFILES:.c=.o}

all: ${PRODUCT}

mandel: mandel.c
	$(CC) $(CFLAGS) -o mandel mandel.c

mandel_omp: mandel_openmp.c
	$(CC) $(CFLAGS) -fopenmp -o mandel_omp mandel_openmp.c

mandel_mpi: mandel_mpi.c
	mpicc -o mandel_mpi mandel_mpi.c $(CFLAGS)

.PHONY: clean

clean:
	rm -f mandel mandel_omp mandel_mpi
