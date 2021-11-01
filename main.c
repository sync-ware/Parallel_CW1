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
	int thread_count;
	int start_index;
	long* thread_id;
} THREAD_ARGS;

MATRIX* make_matrix(int dimension, double init_value, double default_value){
    MATRIX* matrix = (MATRIX*)malloc(sizeof(MATRIX));
	matrix->dimension = dimension;
	matrix->contents = malloc(dimension*sizeof(double*));

	for(int i = 0; i < dimension; i++){
		matrix->contents[i] = malloc(dimension*sizeof(double));

		// if (i == 0){
		// 	for (int j = 0; j < dimension; j++){
		// 		matrix->contents[i][j] = init_value;
		// 	}
		// }else{
		// 	matrix->contents[i][0] = init_value;
		// }
		for (int j = 0; j < dimension; j++){
			if (i == 0 || j == 0){
				matrix->contents[i][j] = init_value;
			}
			else if (i != dimension-1 && j != dimension-1){
				matrix->contents[i][j] = default_value;
			}
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
	// printf("Source: \n");
	// print_matrix(source);
	// printf("Destination: \n");
	// print_matrix(destination);
	//printf("Working on Co-ords: %d, %d\n", x, y);
	if (source->contents[y][x] != destination->contents[y][x]){
		double value = (source->contents[y][x-1] + 
			source->contents[y-1][x] + 
			source->contents[y][x+1] + 
			source->contents[y+1][x]) / 4.0;

		destination->contents[y][x] = value;
		//printf("Changes made.\n");
		return 1;
	} else {
		//printf("Changes already made.\n");
		return 0;
	}
}

int is_matrix_complete(MATRIX* matrix, double precision){
	int goal_count = (matrix->dimension-2)*(matrix->dimension-2);
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
	int s_index = t_args->start_index;
	int max_cells = (t_args->source->dimension-2)*(t_args->source->dimension-2); //4
	while(s_index <= max_cells-1){
		int i = (s_index%(t_args->source->dimension-2))+1;
		int j = (s_index/(t_args->source->dimension-2))+1;
		printf("Thread %ld working on (%d, %d)\n", *t_args->thread_id, j, i);
		process_square(t_args->source, t_args->destination, j, i);
		s_index += t_args->thread_count;
	}
}

THREAD_ARGS* make_thread_args(MATRIX* source, MATRIX* destination, int thread_count, int start_index, long* thread_id){
	THREAD_ARGS* t_args = (THREAD_ARGS*)malloc(sizeof(THREAD_ARGS));
	t_args->source = source;
	t_args->destination = destination;
	t_args->start_index = start_index;
	t_args->thread_count = thread_count;
	t_args->thread_id = thread_id;
	return t_args;
}

int main(void){
	MATRIX* source = make_matrix(10, 2.0, 0.0);
	MATRIX* destination = make_matrix(10, 2.0, -1.0);

	int thread_count = 2;
	pthread_t threads[thread_count];
	for(int c = 0; c < thread_count; c++){
		pthread_create(&threads[c], NULL, thread_process, 
			(void*)make_thread_args(source, destination, thread_count, c, &threads[c]));
	}

	for(int c = 0; c < thread_count; c++){
		pthread_join(threads[c], NULL);
	}

	print_matrix(destination);
	return 0;
}