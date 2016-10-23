NOTE: C++'s STL data structures were used to solve this problem

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

I get the current results:

Test A (input 122)
Graph vertices: 46836 with total edges 1048576. Reached vertices from 122 is 23861 and max level is 9.

Test B (input 100)
Graph vertices: 46836 with total edges 1048576. Reached vertices from 100 is 10 and max level is 4.

Test C (input 201)
Graph vertices: 46836 with total edges 1048576. Reached vertices from 201 is 40428 and max level is 8.

Unfortunately, the sample program given to us does nto give the same results.

Test A (input 100)
Graph vertices: 46836 with total edges 2097152.  Reached vertices from 100 is 46814 and max level is 5
Test B (input 122)
Graph vertices: 46836 with total edges 2097152.  Reached vertices from 201 is 46814 and max level is 5
Test C (input 201)
Graph vertices: 46836 with total edges 2097152.  Reached vertices from 201 is 46814 and max level is 5

As discussed here: https://catalyst.uw.edu/gopost/conversation/renatbek/975025, it gives twice as much for the edges, and it (what I think is) the wrong value for the reached vertices and max depth.

The longest path found for example in Test C, which is of depth 8, is the following:
ROOT -> 0xc9 -> 0xddd3 -> 0xa2b7 -> 0x46c4 -> 0x3ed1 -> 0x329e -> 0x5400 -> 0x1a9f -> done!

This long path above exists in the graph (I have verified so manually by inspecting the input file), therefore it must have been found via BFS. If anyone has seen similar results, I'd love to know.