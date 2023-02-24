#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 1000
#define HEIGHT 1000
#define MAX_ITER 256
#define scale 2
#define xShift 200
#define yShift 0


typedef struct complex{
    double real;
    double imag;
} complex;

int mandelbrot(complex z0);

int main(){
    int grid[WIDTH][HEIGHT];    
    clock_t t;
    t = clock();

    for(int i=0; i<WIDTH; i++){
        for(int j=0; j<HEIGHT; j++){
            complex z0;
            z0.real = ((double)(j-xShift)-(0.5*WIDTH))/(0.5*WIDTH);
            z0.imag = ((double)(i-yShift)-(0.5*HEIGHT))/(0.5*HEIGHT);
            grid[i][j] = 256-mandelbrot(z0);
        }
    }
    t = clock()-t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC;

    FILE* fp = fopen("mandelbrot.pgm", "wb");
    fprintf(fp, "P2\n%d %d\n%d\n", WIDTH, HEIGHT, MAX_ITER);
    for(int i=0; i<WIDTH; i++){
        for(int j=0; j<HEIGHT; j++){
            fprintf(fp, "%d ",grid[i][j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    printf("%f\n", time_taken);
    return 0;
}

int mandelbrot(complex z0){
    
    complex z;
    z.real = 0;
    z.imag = 0;
    int iter = 0;
    // printf("Iter: %d\n", iter);
    while(z.real*z.real + z.imag*z.imag < 4.0 && iter < MAX_ITER){
        
        double temp = z.real*z.real - z.imag*z.imag + z0.real;
        z.imag = 2*z.real*z.imag + z0.imag;
        z.real = temp;
        iter++;
    }
    return iter;
}
