#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <map>
#include <vector>
#include <queue>
#include <stack>
#include <stdbool.h>
#include <pthread.h>

using namespace std;

class Node{
	public:
		unsigned long long id;
		bool visited;
		Node *previous;
		vector<Node*> *children;

		// Used to prevent two threads from marking this node as visited at once
		pthread_mutex_t *lock;

	Node(){
		visited = false;
		previous = NULL;
		children = new vector<Node*>();
		lock = new pthread_mutex_t();
		pthread_mutex_init(lock, NULL);
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

void PrintToFileAndConsole(int verticesTotal, int edgesTotal, unsigned long long rootValue, int vertexCount, int maxLevel){
	printf("\n\nGraph vertices: %d with total edges %d. Reached vertices from %lld is %d and max level is %d.\n\n",
		verticesTotal, edgesTotal, rootValue, vertexCount, maxLevel);
		
	FILE *f = fopen("output.txt", "w+");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    
    fprintf(f, "Graph vertices: %d with total edges %d. Reached vertices from %lld is %d and max level is %d.\n\n",
		verticesTotal, edgesTotal, rootValue, vertexCount, maxLevel);
    
	printf("I wrote this to output.txt\n");
	
    fclose(f);
}

void SingleThreadedBfs(int *maxLevel, int *vertexCount, int *edgeCount, Node *root, Node **deepestNode){
	queue<Node*> *bfsQueue = new queue<Node*>();
	bfsQueue->push(root);
	root->visited = true;
	(*edgeCount)++;
	(*maxLevel) = -1; // Will be incremented to at least 0
	while(bfsQueue->size() > 0){
		printf("Level %d has %lu elements to explore. Exploring...\n", *maxLevel, bfsQueue->size());
		
		queue<Node*> *newQueue = new queue<Node*>();
		
		// Ennumerate through children of elements in the queue
		while(bfsQueue->size() > 0){
			Node *nodeToExplore = bfsQueue->front();
			bfsQueue->pop();
			
			(*vertexCount)++;

			// Keep track of deepest node
			*deepestNode = nodeToExplore;
			
			// Push all children that are not visited
			for (vector<Node*>::iterator it = nodeToExplore->children->begin() ; it != nodeToExplore->children->end(); ++it){
				(*edgeCount)++; // Edges have to be counted, even if we already visited that node

				Node *child = *it;
				// Skip if already visited
				if (!child->visited){
					child->previous = nodeToExplore; // For shortest path
					newQueue->push(child);
					// Mark as visited
					child->visited = true;
				}
			}
		}
		
		free(bfsQueue);
		bfsQueue = newQueue;
		(*maxLevel)++;
	}
	
	free(bfsQueue);
}

struct bfs_work {
	int *maxLevel;
	int *vertexCount;
	int *edgeCount;
	int threadCount;
	queue<Node*> *mainQueue;
	queue<Node*> *partitionedSourceQueueArr;
	queue<Node*> *partitionedDestinationQueueArr;
	pthread_barrier_t *barrier;
	pthread_mutex_t *mainLock;
	bool *workDonePartitionArr;
	bool *workDoneMergeArr;
	int threadId;
	Node **deepestNode;
};

void printBfsWork(struct bfs_work *bfsWork){

}

void *parallelBfsWork(void *arg) {
	int level = 0;
    struct bfs_work *bfsWork = (struct bfs_work *)arg;
	
	printBfsWork(bfsWork);

	//printf("%d : Will start doing work. Main queue size is %lu\n", bfsWork->threadId, bfsWork->mainQueue->size());
	
	// Step 1 - Barrier + partition + barrier
	
	while(bfsWork->mainQueue->size() > 0){
		pthread_barrier_wait(bfsWork->barrier);

		//printf("%d: Made it through barrier!\n", bfsWork->threadId);

		pthread_mutex_lock(bfsWork->mainLock);
		//printf("%d: Acquired lock!\n", bfsWork->threadId);
		if (!bfsWork->workDonePartitionArr[level]){
			printf("%d: Partitioning...\n", bfsWork->threadId);
			int queueSize = bfsWork->mainQueue->size();
			int chunk = queueSize / bfsWork->threadCount;
			//printf("%d: Chunk is %d...\n", bfsWork->threadId, chunk);
			for(int threadIndex = 0; threadIndex < bfsWork->threadCount; threadIndex++){
				int elementsToDequeue;
				if (threadIndex == bfsWork->threadCount - 1){
					elementsToDequeue = queueSize - chunk * (bfsWork->threadCount - 1);
				} else {
					elementsToDequeue = chunk;
				}
				for(int i = 0; i < elementsToDequeue; i++){
					queue<Node*> *localQueue = &bfsWork->partitionedSourceQueueArr[threadIndex];
					Node *node = bfsWork->mainQueue->front();
					bfsWork->mainQueue->pop();
					localQueue->push(node);
				}
			}
		}
		//printf("%d: Done partitioning!\n", bfsWork->threadId);
		bfsWork->workDonePartitionArr[level] = true;
		pthread_mutex_unlock(bfsWork->mainLock);

		//printf("%d: Partitioning step is done\n", bfsWork->threadId);

		queue<Node*> *sourceQueue = &bfsWork->partitionedSourceQueueArr[bfsWork->threadId];
		queue<Node*> *destinationQueue = &bfsWork->partitionedDestinationQueueArr[bfsWork->threadId];
		
		// At this point, we can start individually dequeuing and adding to local new queues
		
		//printf("%d : Main queue has %lu elements, local source has %lu and destination %lu\n", bfsWork->threadId, bfsWork->mainQueue->size(), sourceQueue->size(), destinationQueue->size());
		// Ennumerate through children of elements in the queue
		while(sourceQueue->size() > 0){
			Node *nodeToExplore = sourceQueue->front();
			sourceQueue->pop();
			(*(bfsWork->vertexCount))++; // Vertex are counted uniquely only

			// Keep track of deepest node
			*(bfsWork->deepestNode) = nodeToExplore;
			
			// Push all children
			for (vector<Node*>::iterator it = nodeToExplore->children->begin() ; it != nodeToExplore->children->end(); ++it){
				Node *child = *it;
				(*(bfsWork->edgeCount))++; // Edges have to be counted, even if we already visited that node
				// Skip if already visited
				// Avoid locks as much as possible
				if (!child->visited){
					pthread_mutex_lock(child->lock);
					if (!child->visited){
						child->previous = nodeToExplore; // For shortest path
						destinationQueue->push(child);
						// Mark as visited
						child->visited = true;
					}
					pthread_mutex_unlock(child->lock);
				}
			}
		}
		
		// Barrier to wait for everybody to finish exploring and putting nodes in the queue
		pthread_barrier_wait(bfsWork->barrier);
		pthread_mutex_lock(bfsWork->mainLock);
		//printf("%d: Acquired lock!\n", bfsWork->threadId);
		if (!bfsWork->workDoneMergeArr[level]){
			printf("%d: Merging...\n", bfsWork->threadId);
			for(int threadIndex = 0; threadIndex < bfsWork->threadCount; threadIndex++){
				queue<Node*> *localQueue = &bfsWork->partitionedDestinationQueueArr[threadIndex];
				while(localQueue->size() > 0){
					Node *node = localQueue->front();
					localQueue->pop();
					bfsWork->mainQueue->push(node);
				}
			}
		}
		//printf("%d: Done merging!\n", bfsWork->threadId);
		bfsWork->workDoneMergeArr[level] = true;
		pthread_mutex_unlock(bfsWork->mainLock);

		//printf("%d: Exit merging stage!\n", bfsWork->threadId);
		
		printf("%d : Main queue has %lu elements, local source has %lu and destination %lu\n", bfsWork->threadId, bfsWork->mainQueue->size(), sourceQueue->size(), destinationQueue->size());
		level++;
		*bfsWork->maxLevel = level - 1;
	}
}

void MultiThreadedBfs(int *maxLevel, int *vertexCount, int *edgeCount, Node *root, Node **deepestNode, int threadCount){
	queue<Node*> *mainQueue = new queue<Node*>();
	mainQueue->push(root);
	root->visited = true;
	
	pthread_t *threads = new pthread_t[threadCount]();
	
	queue<Node*> *partitionedSourceQueueArr = new queue<Node*>[threadCount]();
	queue<Node*> *partitionedDestinationQueueArr = new queue<Node*>[threadCount]();
	struct bfs_work *bfsWorkArr = new struct bfs_work[threadCount]();

	pthread_barrier_t *barrier = new pthread_barrier_t();
    pthread_barrier_init(barrier, NULL, threadCount);
	
	pthread_mutex_t *mainLock = new pthread_mutex_t();
	pthread_mutex_init(mainLock, NULL);
	
	// Initialize to false
	bool *workDonePartitionArr = new bool[threadCount];
	bool *workDoneMergeArr = new bool[threadCount];
	for(int i = 0; i < threadCount; i++){
		workDonePartitionArr[i] = false;
		workDoneMergeArr[i] = false;
	}
	
	for(int i = 0; i < threadCount; i++){
		struct bfs_work *bfsWork = &bfsWorkArr[i];
		bfsWork->maxLevel = maxLevel;
		bfsWork->vertexCount = new int();
		bfsWork->edgeCount = new int();
		bfsWork->mainQueue = mainQueue;
		bfsWork->partitionedSourceQueueArr = partitionedSourceQueueArr;
		bfsWork->partitionedDestinationQueueArr = partitionedDestinationQueueArr;
		bfsWork->threadId = i;
		bfsWork->barrier = barrier;
		bfsWork->mainLock = mainLock;
		bfsWork->workDonePartitionArr = workDonePartitionArr;
		bfsWork->workDoneMergeArr = workDoneMergeArr;
		bfsWork->threadCount = threadCount;
		bfsWork->deepestNode = deepestNode;
	}
	
	(*edgeCount)++; // Initial edge
	
	printf("All the work has been calculated, so kicking off threads...\n");
	
	for(int i = 0; i < threadCount; i++){
		struct bfs_work *bfsWork = &bfsWorkArr[i];
		// Create and kick off the thread
		if (pthread_create(&threads[i], NULL, parallelBfsWork, bfsWork) != 0) {
			perror("Failed to create thread\n");
		}
	}
	
	// Join threads after they are done
    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], NULL);
    }
	printf("Threads have been joined...\n");

	// Add up vertex and edge counts
	for(int i = 0; i < threadCount; i++){
		struct bfs_work *bfsWork = &bfsWorkArr[i];
		(*vertexCount) += *(bfsWork-> vertexCount);
		(*edgeCount) += *(bfsWork-> edgeCount);
		delete bfsWork->vertexCount;
		delete bfsWork->edgeCount;
	}

    delete mainQueue;
    delete[] bfsWorkArr;
    delete[] partitionedSourceQueueArr;
    delete[] partitionedDestinationQueueArr;
    delete[] threads;
	
	printf("Parallel work done.\n");
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        printf("usage: %s threads root inputfile\n", argv[0]);
		printf("Sample root: 0");
        return -1;
    }
	
	int threadCount = atoi(argv[1]);
	unsigned long long rootValue = (unsigned long long)atoi(argv[2]);
	char* fileName = argv[3];
	
	printf("Starting. ThreadCount %d, root %llx, inputfile %s\n", threadCount, rootValue, fileName);
    FILE* file = fopen(fileName, "r");

    int buffSize = 100;

    struct edge *buff = (struct edge *) malloc(sizeof(struct edge) * buffSize);
    
    int edgesTotal = 0;
 
	map<unsigned long long, Node*> *graphInputMap = new map<unsigned long long, Node*>();

	
    for (;;) {
        size_t elementsRead = fread(buff, sizeof(struct edge), buffSize, file);
		edgesTotal += elementsRead * 2;
        for(int i = 0; i < elementsRead; i++){
            struct edge edge = buff[i];
			Node *nodeOrigin = GetOrCreateNode(graphInputMap, edge.origin);
			Node *nodeDestination = GetOrCreateNode(graphInputMap, edge.destination);
			nodeOrigin->children->push_back(nodeDestination);
			nodeDestination->children->push_back(nodeOrigin);
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
		exit(1);
	}
	
	int verticesTotal = graphInputMap->size();
	
	printf("I read %d edges and have found %d vertices (in total). Now doing breadth first search to calculate the number of edges and vertices from root 0x%llx \n",
				edgesTotal, verticesTotal, rootValue);

	free(graphInputMap);
	free(buff);
	
	// Now that we've read and loaded all of the elements, we can do breadth first search
	
	int maxLevel = 0; // Assumming at least root exists, level is 1
	int vertexCount = 0;
	int edgeCount = 0;
	
	Node *deepestNode = NULL;
	
	// SingleThreadedBfs(&maxLevel, &vertexCount, &edgeCount, root, &deepestNode);
	MultiThreadedBfs(&maxLevel, &vertexCount, &edgeCount, root, &deepestNode, threadCount);
	
	printf("Here is proof that the max level is %d.\n", maxLevel);
	Node *currentNode = deepestNode;
	stack<Node*> *s = new stack<Node*>();
	while(currentNode != NULL){
		s->push(currentNode);
		currentNode = currentNode->previous;
	}
	printf("A sample path that has this depth is \n");
	printf("ROOT ->");
	while(s->size() > 0){
		Node *next = s->top();
		s-> pop();
		printf(" 0x%llx ->", next->id);
	}
	free(s);
	printf(" done!\n");
	
	PrintToFileAndConsole(verticesTotal, edgesTotal, rootValue, vertexCount, maxLevel);

    return 0;
}