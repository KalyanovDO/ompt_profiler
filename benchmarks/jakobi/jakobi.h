#ifndef __JAKOBI_H
#define __JAKOBI_H


void jakobi_method(float* a, float* b, int n, float err, int end_iter);
float* make_random_a(int n);
float* make_random_b(int n);

//void main_func(int argc, char* argv[]);

#endif