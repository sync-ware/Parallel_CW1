#include <stdio.h>
#include <stdlib.h>

typedef struct matrix
{
    double** contents;
	int dimension;
} MATRIX;

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


void print_matrix(MATRIX* matrix){
    for (int i = 0; i < matrix->dimension; i++){
        for (int j = 0; j < matrix->dimension; j++){
            printf("%f ", matrix->contents[i][j]);
        }
        printf("\n");
    }
	printf("\n");
}

void process_square(MATRIX* matrix, int y, int x){
	int value = (matrix->contents[y][x-1] + 
		matrix->contents[y-1][x] + 
		matrix->contents[y][x+1] + 
		matrix->contents[y+1][x]) / 4.0;

	matrix->contents[y][x] = value;
}

int main(void){
	MATRIX* matrix = make_matrix(4, 2.0);
	//float x[4][4] = (float (*)[4])matrix->contents;
	print_matrix(matrix);
	process_square(matrix, 1, 1);
	print_matrix(matrix);
	free(matrix);
	return 0;
}