#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>

// A square nxn matrix that contains the elements of the matrix and the dimension n.
typedef struct matrix{
    double** contents;
	int dimension;
} MATRIX;

// Structure that holds the arguments we want to pass to the thread process.
typedef struct thread_args{
	MATRIX* source;
	MATRIX* destination;
	int* changes_count;
	int thread_count;
	int start_index;
	long* thread_id;
	int* thread_state;
} THREAD_ARGS;

// Holds the processing state of main.
int processing_active = 1;

int thread_waiting_counter = 0;
int completed_cells = 0;
pthread_mutex_t lock;
pthread_barrier_t barrier;

double precision = 0.01;

// Represents the different states the thread can be in.
enum thread_state{
	thread_active = 1,
	thread_waiting = 2,
	thread_finished = 3
};

// Generate the square matrix structure that has height/width dimension, border values of init_value and inner values of
// default_value
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

// Copies a matrix data structure.
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

// Print a matrix out to the console.
void print_matrix(MATRIX* matrix){
    for (int i = 0; i < matrix->dimension; i++){
        for (int j = 0; j < matrix->dimension; j++){
            printf("%f ", matrix->contents[i][j]);
        }
        printf("\n");
    }
	printf("\n");
}

// Process a value at position (x,y) in the source matrix and write it to the destination matrix.
// Returns 1 if succesful, 0 if the value has already been changed.
int process_square(MATRIX* source, MATRIX* destination, int y, int x){
	double value = (source->contents[y][x-1] + 
		source->contents[y-1][x] + 
		source->contents[y][x+1] + 
		source->contents[y+1][x]) / 4.0;

	if (value-source->contents[y][x] > precision) {
		destination->contents[y][x] = value;
		return 0;
	} else { 
		return 1;
	}
}

// Checks the destination matrix to see if the difference in the values between it and the source matrix is less than
// the precision.
int is_matrix_complete(MATRIX* source, MATRIX* destination){
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

// Thread process that processes individual cells. What cells it proceses depends on the thread index and how many
// threads are active. E.g. if this is the xth thread and there are n threads, it will process each x + n cell index.
// Each index will be converted to 2 dimensions.
void* thread_process(void* args){
	THREAD_ARGS* t_args = (THREAD_ARGS*)args;
	int s_index = 0;
	// Number of cells that can be processed, the area of cells not part of the border.
	int max_cells = (t_args->source->dimension-2)*(t_args->source->dimension-2);
	////printf("Thread %d Started.\n", t_args->start_index);
	// If the thread is active, we can start to process cells.
	while(*t_args->thread_state == thread_active){
		int finished_cells = 0;
		////printf("Thread %d Active.\n", t_args->start_index);
		s_index = t_args->start_index; // Set the start index x, depending on the thread number.
		while(s_index <= max_cells-1){ // While the current index is less than the maximum index.
			// Convert the index to 2 dimensions.
			int j = (s_index%(t_args->source->dimension-2))+1;
			int i = (s_index/(t_args->source->dimension-2))+1;
			////printf("Thread %d working on (%d, %d)\n", t_args->start_index, j, i);
			finished_cells += process_square(t_args->source, t_args->destination, i, j);
			// Increment by the number of threads n, this will be the next cell for the thread to process.
			s_index += t_args->thread_count;
		}

		// Thread will now be placed in a waiting state.
		*t_args->thread_state = thread_waiting;
		pthread_mutex_lock(&lock);
		thread_waiting_counter++;
		completed_cells += finished_cells;
		/*printf("Thread %d has lock. It has completed %d cells. There are %d threads waiting.\n",
			t_args->start_index, finished_cells, thread_waiting_counter);*/
		pthread_mutex_unlock(&lock);
		pthread_barrier_wait(&barrier);
		////printf("Thread %d Waiting.\n", t_args->start_index);
		// While the thread is waiting, the main thread will check if the matrix is complete.
		while(*t_args->thread_state == thread_waiting){ 
			if (!processing_active){ // Check to see if main has stopped processing.
				// Then we can break the while loops by changing the thread state to finished.
				*t_args->thread_state = thread_finished;
				////printf("Thread %d Finished.\n", t_args->start_index);
			}
		}
	}
}

// Generate the thread args structure.
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

void free_matrix(MATRIX* matrix){
	for (int i = 0; i < matrix->dimension; i++){
		
		free(matrix->contents[i]);
		
	}
	free(matrix->contents);
	free(matrix);
}

// Calculate the sum of an integer array.
int array_sum(int arr[], int size){
	int sum = 0;
	for (int x = 0; x < size; x++){
		sum += arr[x];
	}
	return sum;
}

// Find a string in an array of strings. Returns the index of the string if it found, -1 if not found.
int str_array_find(char* arr[], int size, char* string){
	for(int x = 0; x < size; x++){
		if(strcmp(arr[x], string) == 0){
			return x;
		}
	}
	return -1;
}

void print_diag_values(MATRIX* matrix){
	for(int i = 0; i < matrix->dimension; i++){
		for (int j = 0; j < matrix->dimension; j++){
			if (i == j){
				printf("%d,%f\n", i, matrix->contents[i][j]);
			}
		}
	}
}

int main(int argc, char* argv[]){
	// Defaults for arguments
	int matrix_dimension = 4;
	double default_value = 1.0;
	int thread_count = 2;

	// Check to see if we have any arguments.
	int dim_found = str_array_find(argv, argc, "-d");
	int def_val_found = str_array_find(argv, argc, "-v");
	int prec_found = str_array_find(argv, argc, "-p");
	int t_count_found = str_array_find(argv, argc, "-t");

	char *eptr;

	// If we have arguments, start setting up our variables.
	if (dim_found > -1){
		matrix_dimension = atoi(argv[dim_found+1]);
	}
	if (def_val_found > 1){
		default_value = strtod(argv[def_val_found+1], &eptr);
	}
	if (prec_found > -1){
		precision = strtod(argv[prec_found+1], &eptr);
	}
	if (t_count_found > -1){
		thread_count = atoi(argv[t_count_found+1]);
	}

	// To measure performance.
	
	struct timeval start, end;
	gettimeofday(&start, NULL);
	// Set up source matrix.
	MATRIX* source = make_matrix(matrix_dimension, default_value, 0.0);
	// Set up destination, note that all inner values are set to -1, so that they are different to source.
	MATRIX* destination = make_matrix(matrix_dimension, default_value, 0.0);
	int max_cells = (source->dimension-2)*(source->dimension-2);

	if (thread_count > 0){
	// Init threads and their individual states.
		pthread_t threads[thread_count];
		int thread_states[thread_count];

		pthread_mutex_init(&lock, NULL);
		pthread_barrier_init(&barrier, NULL, thread_count+1);

		// Activate each thread, set them all as active.
		for(int c = 0; c < thread_count; c++){
			thread_states[c] = thread_active;
			pthread_create(&threads[c], NULL, thread_process, 
				(void*)make_thread_args(source, destination, thread_count, c, &threads[c], &thread_states[c]));
		}
		
		// While we are processing.
		while(processing_active){
			// Check to see if all threads are in the waiting stage
			pthread_barrier_wait(&barrier);
			if (thread_waiting_counter == thread_count){
				
				// Check to see if the matrix is complete.
				if (max_cells != completed_cells){
					
					// If it's not complete, re-assign our source and destination matrices and set the threads to active.
					free_matrix(source);
					source = copy_matrix(destination);
					
					thread_waiting_counter = 0;
					completed_cells = 0;
					for(int c = 0; c < thread_count; c++){
						thread_states[c] = thread_active;
					}
				} else {
					////printf("Matrix complete.\n");
					
					processing_active = 0; // Stop processing.
				}
			}
		}

		// Join our threads back if we have stopped processing.
		for(int c = 0; c < thread_count; c++){
			pthread_join(threads[c], NULL);
		}

		if (matrix_dimension <= 10){
			print_matrix(destination);
		}
		
		pthread_mutex_destroy(&lock);
		pthread_barrier_destroy(&barrier);
	}else{

		while(completed_cells != max_cells){
			completed_cells = 0;
			for(int i = 1; i < source->dimension-1; i++){
				for(int j = 1; j < source->dimension-1; j++){
					completed_cells+= process_square(source, destination, j, i);
				}
			}
			free_matrix(source);
			source = copy_matrix(destination);
		}
	}
	
	gettimeofday(&end, NULL);
	double seconds = (double) end.tv_sec - start.tv_sec;
	double micros = (double) (end.tv_usec - start.tv_usec)/1000000;
	printf("Matrix size: %d\n", matrix_dimension);
	printf("Initial value: %f\n", default_value);
	printf("Target precision: %f\n", precision);
	printf("Number of threads: %d\n", thread_count);
	printf("Processing time: %f seconds\n", seconds + micros);

	//print_diag_values(source);
	return 0;
}