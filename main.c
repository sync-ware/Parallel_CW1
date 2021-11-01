#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct matrix
{
    double** contents;
	int dimension;
} MATRIX;

typedef struct thread_args
{
	MATRIX* source;
	MATRIX* destination;
	int* changes_count;
} THREAD_ARGS;

MATRIX* make_matrix(int dimension, double init_value){
    MATRIX* matrix = (MATRIX*)malloc(sizeof(MATRIX));
	matrix->dimension = dimension;
	matrix->contents = malloc(dimension*sizeof(double*));

	for(int i = 0; i < dimension; i++){
		matrix->contents[i] = malloc(dimension*sizeof(double));

		if (i == 0){
			for (int j = 0; j < dimension; j++){
				matrix->contents[i][j] = init_value;
			}
		}else{
			matrix->contents[i][0] = init_value;
		}
	}
    return matrix;
}

MATRIX* copy_matrix(MATRIX* matrix){
	MATRIX* copy = (MATRIX*)malloc(sizeof(MATRIX));
	copy->dimension = matrix->dimension;
	copy->contents = malloc(matrix->dimension*sizeof(double*));
	for(int i = 0; i < copy->dimension; i++){
		copy->contents[i] = malloc(copy->dimension*sizeof(double));
		for (int j = 0; j < copy->dimension; j++){
			copy->contents[i][j] = matrix->contents[i][j];
		}
	}
	return copy;
}

void print_matrix(MATRIX* matrix){
    for (int i = 0; i < matrix->dimension; i++){
        for (int j = 0; j < matrix->dimension; j++){
            printf("%f ", matrix->contents[i][j]);
        }
        printf("\n");
    }
	printf("\n");
}

int process_square(MATRIX* source, MATRIX* destination, int y, int x){
	printf("Source: \n");
	print_matrix(source);
	printf("Destination: \n");
	print_matrix(destination);
	printf("Co-ords: %d, %d\n", x, y);
	if (source->contents[y][x] != destination->contents[y][x]){
		int value = (source->contents[y][x-1] + 
			source->contents[y-1][x] + 
			source->contents[y][x+1] + 
			source->contents[y+1][x]) / 4.0;

		destination->contents[y][x] = value;
		return 1;
	} else {
		return 0;
	}
}

int is_matrix_complete(MATRIX* matrix, double precision){
	int goal_count = (matrix->dimension-1)*(matrix->dimension-1);
	int current_count = 0;
	for(int i = 1; i < matrix->dimension-1; i++){
		for (int j = 1; j < matrix->dimension-1; j++){
			if (matrix->contents[i][j] <= precision){
				current_count++;
			}
		}
	}
	return current_count == goal_count;
}

void* thread_process(void* args){
	THREAD_ARGS* t_args = (THREAD_ARGS*)args;
	int max_changes_count = (t_args->source->dimension-1)*(t_args->source->dimension-1);
	while (*t_args->changes_count != max_changes_count)
	{
		printf("Changes Count: %d\n", *t_args->changes_count);
		for(int i = 1; i < t_args->source->dimension-1; i++){
			for (int j = 1; j < t_args->source->dimension-1; j++){
				if(process_square(t_args->source, t_args->destination, i, j)){
					(*t_args->changes_count)++;
				}
			}
		}
		sleep(2);
	}
}

int main(void){
	MATRIX* matrix = make_matrix(4, 2.0);
	MATRIX* destination = copy_matrix(matrix);
	THREAD_ARGS* t_args = (THREAD_ARGS*)malloc(sizeof(THREAD_ARGS));
	t_args->source = matrix;
	t_args->destination = destination;
	int changes_count = 0;
	t_args->changes_count = &changes_count;
	pthread_t th1;
	pthread_t th2;
	pthread_create(&th1, NULL, thread_process, (void*)t_args);
	pthread_create(&th2, NULL, thread_process, (void*)t_args);
	pthread_join(th1, NULL);
	pthread_join(th2, NULL);

	print_matrix(destination);
	return 0;
}