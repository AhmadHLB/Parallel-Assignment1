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
#define proc_size 50

int current_work = 0;

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

    int* recv_buff = malloc(sizeof(int) * proc_size*WIDTH);

    clock_t t;
    t = clock();

    printf("Rank: %d\n", rank);
    
    int count_blocks = HEIGHT/proc_size;

    int* recved = malloc(sizeof(int)*proc_size*WIDTH);

    if(rank == 0){
        MPI_Status status;
        int jobs[size];
        for(int i=1; i<size; i++){
            jobs[i] = current_work;
            arr[current_work*proc_size] = current_work;
            MPI_Send(arr + current_work*proc_size, proc_size*WIDTH, MPI_INT, i, 1, MPI_COMM_WORLD);
            current_work++;
        }

        while(current_work != count_blocks){
            MPI_Recv(recved, proc_size*WIDTH, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int recvedWork = jobs[status.MPI_SOURCE] * proc_size;
            for(int i=0; i<proc_size; i++){
                for(int j=0; j<WIDTH; j++){
                    arr[(i+recvedWork) * WIDTH + j] = recved[i*WIDTH + j];
                }
            }
            arr[current_work*proc_size] = current_work;
            MPI_Send(arr + current_work*proc_size, proc_size*WIDTH, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
            jobs[status.MPI_SOURCE] = current_work;
            current_work++;
        }

        for(int i=1; i<size; i++){
            MPI_Recv(recved, proc_size*WIDTH, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int recvedWork = jobs[status.MPI_SOURCE] * proc_size;
            for(int i=0; i<proc_size; i++){
                for(int j=0; j<WIDTH; j++){
                    arr[(i+recvedWork) * WIDTH + j] = recved[i*WIDTH + j];
                }
            }
        }

        for(int i=1; i<size; i++){
            MPI_Send(0, 0, MPI_INT, i, 2, MPI_COMM_WORLD);
        }
    }
    else{
        MPI_Status status;
        while(1){
            MPI_Recv(recved, proc_size*WIDTH, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if(status.MPI_TAG == 2){
                break;
            }

            int curr = recved[0];
            // printf("%d\n", curr);

            for(int i=0; i<proc_size; i++){
                for(int j=0;  j<WIDTH; j++){
                    complex z0;
                    z0.real = ((double)(j-xShift)-(0.5*WIDTH))/(0.5*WIDTH);
                    z0.imag = ((double)((i+curr*proc_size)-yShift)-(0.5*HEIGHT))/(0.5*HEIGHT);
                    recved[i*WIDTH + j] = mandelbrot(z0);
                }
            }

            MPI_Send(recved, proc_size*WIDTH, MPI_INT, 0, 1, MPI_COMM_WORLD);
            
        }
    }


    t = clock()-t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC;
    if(rank == 0){
        FILE* fp = fopen("mandelbrot3.pgm", "wb");
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


    MPI_Finalize();
    return 0;
}






