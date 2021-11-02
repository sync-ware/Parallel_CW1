#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

int str_to_int(char* string){
	int length = strlen(string);
	int output = 0;
	for(int x = 0; x < length; x++){
		output += (string[x] - 48)*((int)pow(10, (length-1)-x));
	}
	return output;
}

int main(void){
    printf("%d\n", str_to_int("2423234"));
    return 0;
}
