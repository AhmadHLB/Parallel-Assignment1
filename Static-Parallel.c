#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define WIDTH 1000
#define HEIGHT 1000
#define MAX_ITER 1000
#define xShift 200
#define yShift 0

typedef struct complex{
    double real;
    double imag;
} complex;



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

int main(int argc, char** argv)
{
    
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int* arr = malloc(sizeof(int)*HEIGHT*WIDTH);
    if(rank == 0){

        for(int i=0; i<HEIGHT; i++){
            for(int j=0; j<WIDTH; j++){
                arr[i*WIDTH + j] = 0;
                
            }
        }
    }

    int proc_size= HEIGHT/size;
    int* recv_buff = malloc(sizeof(int) * proc_size*WIDTH);

    clock_t t;
    t = clock();
    printf("%d\n", rank);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Scatter(arr, proc_size*WIDTH, MPI_INT, recv_buff, proc_size*WIDTH,  MPI_INT, 0, MPI_COMM_WORLD);

   
    for(int i=0; i<proc_size; i++){
        for(int j=0;  j<WIDTH; j++){
            complex z0;
            z0.real = ((double)(j-xShift)-(0.5*WIDTH))/(0.5*WIDTH);
            z0.imag = ((double)((i+rank*proc_size)-yShift)-(0.5*HEIGHT))/(0.5*HEIGHT);
            // printf("%d ", recv_buff[i*WIDTH + j]);
            recv_buff[i*WIDTH + j] = mandelbrot(z0);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Gather(recv_buff, proc_size*WIDTH, MPI_INT, arr, proc_size*WIDTH, MPI_INT, 0, MPI_COMM_WORLD);
    t = clock()-t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC;
    if(rank == 0){
        FILE* fp = fopen("mandelbrot2.pgm", "wb");
        fprintf(fp, "P2\n%d %d\n%d\n", WIDTH, HEIGHT, MAX_ITER);
        for(int i=0; i<WIDTH; i++){
            for(int j=0; j<HEIGHT; j++){
                fprintf(fp, "%d ",arr[i*WIDTH + j]);
            }
            fprintf(fp, "\n");
        }
        fclose(fp);
        printf("%f\n", time_taken);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    return 0;
}

// mpirun -np 5 -hostfile machinefile --mca btl_tcp_if_include 192.168.10.0/24 ./Static-Parallel
