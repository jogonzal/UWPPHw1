#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <map>
#include <vector>
#include <queue>
#include <stack>
#include <stdbool.h>

using namespace std;

class Node{
	public:
		unsigned long long id;
		bool visited;
		// LOCK IS MISSING
		Node *previous;
		vector<Node*> *children;
	
	Node(){
		visited = false;
		previous = NULL;
		children = new vector<Node*>();
	}
};

struct edge{
    unsigned long long origin;
    unsigned long long destination;
};

void printEdge(struct edge edge){
    printf("%llx -> %llx\n", edge.origin, edge.destination);
}

Node* GetOrCreateNode(map<unsigned long long, Node*> *graphInputMap, unsigned long long id){
	Node *node;
	std::map<unsigned long long, Node*>::iterator it;
	it = graphInputMap->find(id);
	if (it == graphInputMap->end()){
		//printf("Does not contain\n");
		node = new Node();
		node->id = id;
		(*graphInputMap)[id] = node;
		return node;
	} else {
		//printf("Contains\n");
		// This would mean the dictionary already contained the node
		// simply return it
		return it->second;
	}
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        printf("usage: %s threads target\n", argv[0]);
		printf("Sample target: 26849");
        return -1;
    }
	
	int threadCount = atoi(argv[1]);
	unsigned long long rootValue = (unsigned long long)atoi(argv[2]);
	
	printf("Starting. ThreadCount %d, root %llx\n", threadCount, rootValue);
    char const* const fileName = "graphinput.bin";
    FILE* file = fopen(fileName, "r");

    int buffSize = 100;

    struct edge *buff = (struct edge *) malloc(sizeof(struct edge) * buffSize);
    
    int edgesRead = 0;
 
	map<unsigned long long, Node*> *graphInputMap = new map<unsigned long long, Node*>();

	
    for (;;) {
        size_t elementsRead = fread(buff, sizeof(struct edge), buffSize, file);
		edgesRead += elementsRead;
        for(int i = 0; i < elementsRead; i++){
            struct edge edge = buff[i];
			Node *nodeOrigin = GetOrCreateNode(graphInputMap, edge.origin);
			Node *nodeDestination = GetOrCreateNode(graphInputMap, edge.destination);
			nodeOrigin->children->push_back(nodeDestination);
		}
        if (elementsRead < buffSize) { break; }
    }
	
	fclose(file);
	
	// Get the pointer to the root
	Node *root = NULL;
	std::map<unsigned long long, Node*>::iterator it;
	it = graphInputMap->find(rootValue);
	if (it != graphInputMap->end()){
		printf("The root you specified was in the input file. Now calculating stats...\n");
		root = it->second;
	} else {
		printf("Where did you get this value from? The root you specified wasn't in the input file\n");
	}
	
	printf("I read %d edges and have found %lu vertices (in total). Now doing breadth first search to calculate the number of edges and vertices from root \n",
				edgesRead, graphInputMap->size());
				
	free(graphInputMap);
	free(buff);
	
	// Now that we've read and loaded all of the elements, we can do breadth first search
	
	int maxLevel = 0; // Assumming at least root exists, level is 1
	int vertexCount = 0;
	int edgeCount = 0;
	queue<Node*> *bfsQueue = new queue<Node*>();
	bfsQueue->push(root);
	while(bfsQueue->size() > 0){
		printf("Level %d has %lu elements to explore. Exploring...\n", maxLevel, bfsQueue->size());
		
		queue<Node*> *newQueue = new queue<Node*>();
		
		// Ennumerate through children of elements in the queue
		while(bfsQueue->size() > 0){
			Node *nodeToExplore = bfsQueue->front();
			bfsQueue->pop();
			
			edgeCount++; // Edges have to be counted, even if we already visited that node

			// Skip if already visited
			if (!nodeToExplore->visited){
				vertexCount++;
				
				// Push all children
				for (vector<Node*>::iterator it = nodeToExplore->children->begin() ; it != nodeToExplore->children->end(); ++it){
					Node *child = *it;
					child->previous = nodeToExplore; // For shortest path
					newQueue->push(child);
				}
				
				// Mark as visited
				nodeToExplore->visited = true;
			}
		}
		
		free(bfsQueue);
		bfsQueue = newQueue;
		maxLevel++;
	}

	printf("Found %d vertices, %d maxLevel, %d edges.\n", vertexCount, maxLevel, edgeCount);
	
	free(bfsQueue);

    return 0;
}