#include <stdio.h>
#include <stdlib.h>

typedef struct matrix
{
    float** contents;
	int width;
	int height;
} MATRIX;

MATRIX* make_matrix(int width, int height, float x_axis_val, float y_axis_val){
    MATRIX* matrix = (MATRIX*)malloc(sizeof(MATRIX));
	matrix->height = height;
	matrix->width = width;
	matrix->contents = malloc(height*sizeof(float*));

	for(int i = 0; i < height; i++){
		matrix->contents[i] = malloc(width*sizeof(float));
		for (int j = 0; j < width; j++){
			matrix->contents[i][j] = 1.0;
		}
	}
    return matrix;
}


void print_matrix(float** matrix){
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            printf("%f ", matrix[i][j]);
        }
        printf("\n");
    }
}

int main(void){
    MATRIX* matrix = make_matrix(4,4);
	//float x[4][4] = (float (*)[4])matrix->contents;
    print_matrix(matrix->contents);
	free(matrix);
    return 0;
}