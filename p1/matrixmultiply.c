#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

void printRow(int columns, int *arr){
    printf("[");
    for(int c = 0; c < columns; c++)
    {
        int element = arr[c];
        printf("%d\t", element);
    }
    printf("]\n");
}

void printInputParameters(int inputMatrixRows, int inputMatrixColumns, int inputVectorRows, int **matrix, int *vector){
    printf("\n The matrix is %d x %d\n", inputMatrixRows, inputMatrixColumns);
    for(int r = 0; r < inputMatrixRows; r++){
        printRow(inputMatrixColumns, matrix[r]);
    }
    printf("\nThe vector is %d x 1\n", inputVectorRows);
    printRow(inputVectorRows, vector);
}

void singleThreadedMatrixMultiply(int inputMatrixRows, int inputMatrixColumns, int **matrix, int *vector, int *result){
    for(int r = 0; r < inputMatrixRows; r++){
        int *rowToMultiply = matrix[r];
        int acc = 0;
        for(int c = 0; c < inputMatrixColumns; c++){
            int localResult = rowToMultiply[c] * vector[c];
            acc = acc + localResult;
        }
        result[r] = acc;
    }
}

void writeArrayToFile(int columns, int *arr){
    FILE *f = fopen("output.txt", "w+");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    
    for(int c = 0; c < columns; c++)
    {
        int element = arr[c];
        if (c == columns - 1){
            fprintf(f, "%d", element);
        } else {
            fprintf(f, "%d,", element);
        }
    }
    
    fclose(f);
}

int main(int argc, char* argv[])
{
    char const* const fileName = "input.txt"; /* should check that argc > 1 */
    FILE* file = fopen(fileName, "r"); /* should check the result */
    char line[1024];

    int inputMatrixRows = -1;
    int inputMatrixColumns = -1;
    bool arraysAreInitialized = false;
    bool matrixFilled = false;
    bool vectorFilled = false;
    
    int matrixOffset = 0;
    int vectorOffset = 0;
    int expectedMatrixElements = -1;
    int expectedVectorElements = -1;
    
    int **matrix;
    int *vector;

    while (fgets(line, sizeof(line), file)) {
        char *element = strtok(line, ",");
        while (element != NULL){
            int elementAsInteger = atoi(element);

            //printf("%d\n", elementAsInteger);
            
            // State machine goes here
            if (inputMatrixRows == -1){
                inputMatrixRows = elementAsInteger;
            } else if (inputMatrixColumns == -1){
                inputMatrixColumns = elementAsInteger;
            } else {
                if (!arraysAreInitialized){
                    printf("Dimensions are %d x %d. Creating arrays...\n", inputMatrixRows, inputMatrixColumns);
                    vector = (int *) malloc(sizeof(int) * inputMatrixColumns);
                    matrix = (int **) malloc(sizeof(int*) * inputMatrixRows);
                    for(int i = 0; i < inputMatrixRows; i++){
                        matrix[i] = (int *) malloc(sizeof(int) * inputMatrixColumns);
                    }
                    expectedMatrixElements = inputMatrixRows * inputMatrixColumns;
                    expectedVectorElements = inputMatrixColumns;
                    arraysAreInitialized = true;
                }
                
                
                if (!matrixFilled){
                    int r = matrixOffset/inputMatrixColumns;
                    int c = matrixOffset%inputMatrixColumns;
                    matrix[r][c] = elementAsInteger;
                    matrixOffset++;
                    if (matrixOffset == expectedMatrixElements){
                        matrixFilled = true;
                    }
                } else if (!vectorFilled){
                    vector[vectorOffset] = elementAsInteger;
                    vectorOffset++;
                    if (vectorOffset == expectedVectorElements){
                        vectorFilled = true;
                    }
                } else {
                    printf("\nERROR IN INPUT!!!!!!!\nToo many numbers?");
                    return -1;
                }
            }
            
            // Advance to next
            element = strtok(NULL, ",");
        }
    }
    
    if (!(matrixFilled && vectorFilled)){
        printf("\nERROR IN INPUT!!!!!!!\nNot enough numbers?");
        return -1;
    }
    
    int inputVectorRows = inputMatrixColumns;
    printInputParameters(inputMatrixRows, inputMatrixColumns, inputVectorRows, matrix, vector);
    fclose(file);

    // At this point, we are done reading from the file. We will perform matrix multiply now
    
    // Matrix multiply - grab the entire vector and the ith row, then multiply and sum one by one
    int *result = (int*) malloc(sizeof(int) * inputVectorRows);
    
    singleThreadedMatrixMultiply(inputMatrixRows, inputMatrixColumns, matrix, vector, result);
    
    printf("The resulting vector is\n");
    printRow(inputVectorRows, result);
    
    writeArrayToFile(inputMatrixColumns, result);
    
    free(vector);
    free(result);
    for(int i = 0; i < inputMatrixRows; i++){
        free(matrix[i]);
    }
    free(matrix);
    
    return 0;
}