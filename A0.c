#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct FieldStruct {
	int Field1;

} FieldStruct;

FieldStruct *array ;
FILE *fp;

int main (void){
	array = malloc(sizeof(FieldStruct) *10);
	fp = fopen("FinalFileA0.bin", "w+b");
	int i;
	for(i =0 ; i<10 ; i++){
		array[i].Field1 = i;
		fwrite(&array[i], sizeof(array) , 1 , fp);
	}

fclose(fp);
}
