#ifndef MANDEL_H
#define MANDEL_H

namespace mandel 
{
    void mandel(int width, int height);
    void mandel_omp(int width, int height, int num_threads);
}

#endif