#include <iostream>
#include "mandel.h"

int main()
{
    int width, height, num_threads;

    printf("Enter the width of the image, recommend 800, increase to increase image resolution: ");
    scanf("%d", &width);
    printf("Enter the height of the image, recommend 600, increase to increase image resolution: ");
    scanf("%d", &height);
    printf("Enter the number of threads to use for OpenMP calculation: ");
    scanf("%d", &num_threads);

    mandel::mandel(width, height);
    mandel::mandel_omp(width, height, num_threads);
}