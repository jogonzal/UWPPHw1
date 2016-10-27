NOTE: C++'s STL data structures were used to solve this problem (queue, stack, map)

Strategy to parallelize
For the BFS, we have a node object with a value, as well child nodes attached to it. The strategy without parallelizing would be:

1. Put root into a queue
2. Dequeue a node
2. Enqueue all of the node's children if they are not visited, marking them as visited
3. Go to (2) until there are no nodes in queue
4. Done! (don't forget to count edges, vertices, levels)

When parallelizing, I decided to parallelize step (2) and create a "merge" and "partition" stage

1. Put root into a queue
2. (Barrier required) PARTITION: partition the queue into equal parts for each thread. 
3. Each thread looks at its queue nodes and enqueues their children if they are not visited
	Note: A lock per node is required to avoid race conditions on the visited flag on each node
4. (Barrier required) MERGE: merge the separate queues into one only queue
5. Go to step (2) if the main queue is not empty
4. Done! (don't forget to count edges, vertices, levels)
	Levels is simple (simply number of times (2) was executed) Vertices is can be calculated locally on each thread and them summed up together in the end

Sample results

I get the current results, just like the sample problem.

Test A (input 100)
Graph vertices: 46836 with total edges 2097152.  Reached vertices from 100 is 46814 and max level is 5
Test B (input 122)Graph vertices: 46836 with total edges 2097152.  Reached vertices from 201 is 46814 and max level is 5
Test C (input 201)
Graph vertices: 46836 with total edges 2097152.  Reached vertices from 201 is 46814 and max level is 5
Test D (input 3613)
Graph vertices: 46836 with total edges 2097152.  Reached vertices from 3613 is 2 and max level is 1

Sample call
make
./main threads root inputfile
 
./main 2 200 graphinput.bin