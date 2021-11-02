#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <math.h>

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
	int* thread_state;
} THREAD_ARGS;

int processing_active = 1;

enum thread_state{
	thread_active = 1,
	thread_waiting = 2,
	thread_finished = 3
};

MATRIX* make_matrix(int dimension, double init_value, double default_value){
    MATRIX* matrix = (MATRIX*)malloc(sizeof(MATRIX));
	matrix->dimension = dimension;
	matrix->contents = malloc(dimension*sizeof(double*));

	for(int i = 0; i < dimension; i++){
		matrix->contents[i] = malloc(dimension*sizeof(double));

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

//int threads_finished = 0;

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

int is_matrix_complete(MATRIX* source, MATRIX* destination, double precision){
	int goal_count = (source->dimension-2)*(source->dimension-2);
	int current_count = 0;
	for(int i = 1; i < source->dimension-1; i++){
		for (int j = 1; j < source->dimension-1; j++){
			if ((destination->contents[i][j] - source->contents[i][j]) <= precision){
				current_count++;
			}
		}
	}
	return current_count == goal_count;
}

void* thread_process(void* args){
	THREAD_ARGS* t_args = (THREAD_ARGS*)args;
	int s_index = 0;
	int max_cells = (t_args->source->dimension-2)*(t_args->source->dimension-2);
	printf("Thread %ld Started.\n", *t_args->thread_id);
	while(*t_args->thread_state == thread_active){
		printf("Thread %ld Active.\n", *t_args->thread_id);
		s_index = t_args->start_index;
		while(s_index <= max_cells-1){
			int i = (s_index%(t_args->source->dimension-2))+1;
			int j = (s_index/(t_args->source->dimension-2))+1;
			//printf("Thread %ld working on (%d, %d)\n", *t_args->thread_id, j, i);
			process_square(t_args->source, t_args->destination, j, i);
			s_index += t_args->thread_count;
		}

		*t_args->thread_state = thread_waiting;
		printf("Thread %ld Waiting.\n", *t_args->thread_id);
		while(*t_args->thread_state == thread_waiting){ 
			if (!processing_active){
				*t_args->thread_state = thread_finished;
				printf("Thread %ld Finished.\n", *t_args->thread_id);
			}
		}
	}
}

THREAD_ARGS* make_thread_args(MATRIX* source, MATRIX* destination, int thread_count, int start_index, long* thread_id,
	int* thread_state){
	THREAD_ARGS* t_args = (THREAD_ARGS*)malloc(sizeof(THREAD_ARGS));
	t_args->source = source;
	t_args->destination = destination;
	t_args->start_index = start_index;
	t_args->thread_count = thread_count;
	t_args->thread_id = thread_id;
	t_args->thread_state = thread_state;
	return t_args;
}

int array_sum(int arr[], int size){
	int sum = 0;
	for (int x = 0; x < size; x++){
		sum += arr[x];
	}
	return sum;
}

int str_array_find(char* arr[], int size, char* string){
	for(int x = 0; x < size; x++){
		if(strcmp(arr[x], string) == 0){
			return x;
		}
	}
	return -1;
}

int str_to_int(char* string){
	int length = strlen(string);
	int output = 0;
	for(int x = 0; x < length; x++){
		output += (string[x] - 48)*((int)pow(10, (length-1)-x));
	}
	return output;
}

int main(int argc, char* argv[]){

	clock_t t;
	t = clock();

	int matrix_dimension = 4;
	double default_value = 1.0;
	double precision = 0.01;
	int thread_count = 2;

	MATRIX* source = make_matrix(matrix_dimension, default_value, 0.0);
	MATRIX* destination = make_matrix(matrix_dimension, default_value, -1.0);

	
	pthread_t threads[thread_count];
	int thread_states[thread_count];

	for(int c = 0; c < thread_count; c++){
		thread_states[c] = thread_active;
		pthread_create(&threads[c], NULL, thread_process, 
			(void*)make_thread_args(source, destination, thread_count, c, &threads[c], &thread_states[c]));
	}
	
	while(processing_active){
		if (array_sum(thread_states, thread_count) == thread_count*thread_waiting){
			printf("Checking Matrix.\n");
			//print_matrix(destination);
			//sleep(2);
			if (!is_matrix_complete(source, destination, precision)){
				free(source);
				source = copy_matrix(destination);
				free(destination);
				destination = make_matrix(matrix_dimension, default_value, -1.0);
				for(int c = 0; c < thread_count; c++){
					thread_states[c] = thread_active;
				}
			} else {
				printf("Matrix complete.\n");
				processing_active = 0;
			}
		}
	}

	for(int c = 0; c < thread_count; c++){
		pthread_join(threads[c], NULL);
	}

	//print_matrix(destination);

	t = clock() - t;
	double time_taken = ((double)t)/CLOCKS_PER_SEC; // calculate the elapsed time
	printf("The program took %f seconds to execute.\n", time_taken);

	return 0;
}