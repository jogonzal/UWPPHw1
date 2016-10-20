#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <map>
#include <vector>
#include <queue>
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
	printf("Starting\n");
    char const* const fileName = "graphinput.bin";
    FILE* file = fopen(fileName, "r");

    int buffSize = 100;

    struct edge *buff = (struct edge *) malloc(sizeof(struct edge) * buffSize);
    
    int edgesRead = 0;
 
 	printf("Initializing map\n");
	map<unsigned long long, Node*> *graphInputMap = new map<unsigned long long, Node*>();

	Node *root = NULL;
	
    for (;;) {
        size_t elementsRead = fread(buff, sizeof(struct edge), buffSize, file);
		edgesRead += elementsRead;
        for(int i = 0; i < elementsRead; i++){
            struct edge edge = buff[i];
			Node *nodeOrigin = GetOrCreateNode(graphInputMap, edge.origin);
			Node *nodeDestination = GetOrCreateNode(graphInputMap, edge.destination);
			nodeOrigin->children->push_back(nodeDestination);
			
			if (root == NULL){
				root = nodeOrigin;
			}
		}
        if (elementsRead < buffSize) { break; }
    }
	// Now that we've read and loaded all of the elements, we can do breadth first search

	printf("I read %d edges and have found %lu nodes. Now doing breadth first search\n", edgesRead, graphInputMap->size());

	int level = 0;
	queue<Node*> *bfsQueue = new queue<Node*>();
	bfsQueue->push(root);
	while(bfsQueue->size() > 0){
		printf("Level %d has %lu elements. Exploring...\n", level, bfsQueue->size());
		
		queue<Node*> *newQueue = new queue<Node*>();
		
		// Ennumerate through children of elements in the queue
		while(bfsQueue->size() > 0){
			Node *nodeToExplore = bfsQueue->front();
			bfsQueue->pop();
			// Skip if already visited
			if (!nodeToExplore->visited){
				// Push all children if not visited
				for (vector<Node*>::iterator it = nodeToExplore->children->begin() ; it != nodeToExplore->children->end(); ++it){
					Node *child = *it;
					child->previous = nodeToExplore; // For shortest path
					newQueue->push(child);
				}
				nodeToExplore->visited = true;
			}
		}
		
		free(bfsQueue);
		bfsQueue = newQueue;
		level++;
	}


	fclose(file);
    free(buff);
	
    return 0;
}