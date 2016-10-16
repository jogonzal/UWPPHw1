#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct edge{
    unsigned long long origin;
    unsigned long long destination;
};

void printEdge(struct edge edge){
    printf("%llx -> %llx\n", edge.origin, edge.destination);
}

int main(int argc, char* argv[])
{
    char const* const fileName = "graphinput.bin";
    FILE* file = fopen(fileName, "r");

    int buffSize = 100;

    struct edge *buff = malloc(sizeof(struct edge) * buffSize);
    
    int edgesRead = 0;
    
    for (;;) {
        size_t elementsRead = fread(buff, sizeof(struct edge), buffSize, file);
		edgesRead += elementsRead;
        for(int i = 0; i < elementsRead; i++){
            struct edge edge = buff[i];
        }
        if (elementsRead < buffSize) { break; }
    }
	
	printf("I read %d edges. Now doing breadth first search\n", edgesRead);
	
	// Now that we've read and loaded all of the elements, we can do breadth first search

	fclose(file);
    free(buff);
	
    return 0;
}