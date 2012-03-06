#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define SILENT 0
#define NORMAL 1
#define VERBOSE 2
#define DEBUG 3

#define CRITICAL 0
#define ALERT 1
#define INFO 2

/* I really like to use boolean values */
//#define bool char
#define true 1
#define false 0


/******* The output macros **********/

#define GPRINT(level,out,args...) if(verbosity >= level) fprintf(out,args) //Generic print macro
#define EPRINT(level,args...) GPRINT(level,stderr,args) //Prints message to stderr if level is <= verbosity
#define PRINT(level,args...) GPRINT(level,stdout,args) //Prints to stdout of level <= verbosity



/******* The structures the graph will be built upon *********/
  
/**
  Both nodes and edges are built as linked lists to support an
  (theoretical) infinite number of each. 
  Each node has a uniqe id of a single char. Each node also 
  has an arbirtaty number of edges to other nodes.
  Edges are one way connections between nodes. Each edge has 
  a "weight", i.e. the cost of traveling along that edge.
**/
struct struct_node {
	char id; 
	int no_edges;
	struct struct_edge * edges;
	struct struct_node * next;
};

struct struct_edge {
	int id;
	int weight;
	struct struct_node * endpoint;
	struct struct_edge * next;
};

typedef struct struct_node node;
typedef struct struct_edge edge;
typedef char bool;

FILE * graphFile;
int edgeid = 1;
int noNodes = 0;
int verbosity = NORMAL;
node * allthenodesHead;
node * first = NULL; //the first node inte the node list, i.e. the node with the lowest id
node * entrypoint = NULL; // the entry point to the graph, i.e. the first node to be created
int bestPathCost = -1;


int insideTravelGraph(node * visitedNodes[], int noVisited, int costSoFar, node * currNode, node * solution[]);

/******* Functions for graph management **********/

node * newNode(char name) {
	noNodes++;
	EPRINT(DEBUG,"Creating new node %c. Now have %d nodes\n",name,noNodes);
	node * curr;
	curr = (node *)malloc(sizeof(node));
	curr->no_edges = 0;
	curr->edges = NULL;
	curr->next = NULL;
	curr->id = name;
	return curr;
}


/**
  getNode takes a char with the ID of the node to look for
  and returns the node pointer for that node.
  If the node is in the node list already that node pointer will be 
  returned. If the node is not already in the list a new node will 
  be created and inserted in the list, then returned.
**/
node * getNode(char cNode) {		//Adds a new node to the list of nodes
	node * newnode;
	if(entrypoint == NULL) {		//First node
		EPRINT(DEBUG,"Adding %c as entry point\n",cNode);
		newnode = newNode(cNode);	//Create node
		entrypoint = newnode;		//Entrypoint will never change after this initial set
		first = newnode; 			//First is the first node in the node list
		allthenodesHead = newnode;
	}
	else { //There are already some nodes in the list
		node * curr = first;
		if(cNode < curr->id) { //Should the node be first in the list?
			EPRINT(DEBUG,"Adding %c to node list\n",cNode);
			newnode = newNode(cNode);
			newnode->next = first;
			first = newnode;
		}
		else if(cNode == curr->id) { //Is cNode and first the same node?
			EPRINT(DEBUG,"Node %c already in node list\n",cNode);
			newnode = curr;
		}
		else { //So where should it be then?
			while((curr->next != NULL) && (curr->next->id < cNode)) {
				EPRINT(DEBUG,"Node %c already in node list\n",cNode);
				curr = curr->next;
			}
			if(curr->next != NULL && curr->next->id == cNode) { //Does this node already exist?
				newnode = curr->next; //Set curr->next to be returned
			}
			else { //Insert node into list
				EPRINT(DEBUG,"Adding %c to node list\n",cNode);
				newnode = newNode(cNode);
				newnode->next = curr->next;
				curr->next = newnode;
			}
		}
	}
	return newnode;
}

void addToEdgeList(node * n, edge * e) {
	if(n->no_edges==0) { //No edges on this node yet.
		EPRINT(DEBUG,"Setting first edge on %c\n", n->id);
		n->edges = e;
	}
	else {
		EPRINT(DEBUG,"Adding another edge to node %c\n",n->id);
		edge * edgeptr = n->edges;
		while(edgeptr->next) edgeptr = edgeptr->next; //Find the last edge in the list
		edgeptr->next = e;
	}
	n->no_edges++;
}

void addEdges(node * endpoint1, node * endpoint2, int weight) {
	//Add edge one way
	edge * e;
	e=(edge *)malloc(sizeof(edge));
	e->id = edgeid++;
	e->weight = weight;
	e->endpoint = endpoint2;
	e->next = NULL;
	addToEdgeList(endpoint1, e);

	//Add edge the other way
	e=(edge *)malloc(sizeof(edge));
	e->id = edgeid++;
	e->weight = weight;
	e->endpoint = endpoint1;
	e->next = NULL;
	addToEdgeList(endpoint2, e);
	
}

void listEdges(char cNode) {
	node * n = getNode(cNode);
	edge * curr = n->edges;
	printf("Node %c:",n->id);
	while(curr != NULL) {
		printf(" %c-%d",curr->endpoint->id,curr->weight);
		curr = curr->next;
	}
	printf("\n");
}

void removeEdges(edge * e) {
	edge * curr = e;
	edge * next;
	while(curr != NULL) {
		next = curr->next;
		free(curr);
		curr = next;
	}
}

void removeNode(node * n) {
	removeEdges(n->edges);
	free(n);
}

void freeAllNodes(node * start) {
	node * curr = start;
	node * next;
	while(curr != NULL) {
		next = curr->next;
		EPRINT(DEBUG,"Freeing node %c - 0x%08x\n",curr->id, curr);
		removeNode(curr);
		curr = next;
	}
}


bool visit(node * currNode, node * visitedNodes[], int * noVisited) {
	EPRINT(DEBUG,"Visit: currNode: %c, noVisited: %d, noNodes: %d\n",currNode->id, *noVisited, noNodes);
	if(*noVisited > noNodes) {
		return false;
	}
	if(*noVisited == noNodes) {
		EPRINT(DEBUG,"Last node... ");
		if(currNode == entrypoint) {
			EPRINT(DEBUG,"%c is the goal. Ok\n", currNode->id);
			(*noVisited)++;
			return true;
		}
		else {
			EPRINT(DEBUG,"%c isn't the goal. Failing\n", currNode->id);
			return false;
		}
	}
	else {
		for(int i=0;i<*noVisited;i++) {
			EPRINT(DEBUG,"Comparing %c to %c... ", currNode->id, visitedNodes[i]->id);
			if(currNode == visitedNodes[i]) {
				EPRINT(DEBUG,"match, can't visit again\n");
				return false;
			}
			EPRINT(DEBUG,"no match\n");
		}
		visitedNodes[*noVisited] = currNode;
		(*noVisited)++;
		EPRINT(DEBUG,"%c haven't been visited yet\n",currNode->id);
		return true;
	}
	EPRINT(CRITICAL,"Reached end of visit(). This shouldn't be possible!\n");
	return false;
}

int travelGraph() {
	node * solution[noNodes+1];
	node * visitedNodes[noNodes];
	int shortestPath = -1;

	for(int i=0;i<noNodes;i++) { //Zero out visited array
		visitedNodes[i] = 0;
	}
	shortestPath = insideTravelGraph(visitedNodes, 0, 0, entrypoint, solution);
	if(shortestPath >= 0) {
		solution[0] = entrypoint;

		//Testing things
//		node * curr = first;
//		for(int i=0;i<noNodes+1; i++) {
//			printf("%c at %08x -- %c resides at %08x\n",solution[i] != NULL ? solution[i]->id : '?', solution[i], curr != NULL ? curr->id : '?', curr);
//			if(curr != NULL) curr = curr->next;
//		}

		for(int i=0;i<=noNodes;i++) { //Notice the less than or EQUAL
			PRINT(CRITICAL, " %c", solution[i]->id);
		}
		PRINT(CRITICAL, ": %d\n",shortestPath);
	}
	else {
		PRINT(CRITICAL, "Graph unsolvable :(\n");
	}
	return shortestPath;
}

int insideTravelGraph(node * visitedNodes[], int noVisited, int costSoFar, node * currNode, node * solution[]) {
	if(visit(currNode, visitedNodes, &noVisited)) {
		if(noVisited > noNodes) {
			return costSoFar;
		}
		else {
			edge * bestPath = NULL;
			edge * e = currNode->edges;
			while(e != NULL) { 
				int thisPathCost = insideTravelGraph(visitedNodes, noVisited, costSoFar+e->weight, e->endpoint, solution);
				if(thisPathCost >= 0) {
					if(bestPathCost < 0 || thisPathCost <= bestPathCost) {
						bestPathCost = thisPathCost;
						bestPath = e;
					}
				}
				e = e->next;
			}
//			printf("Done traversing edges. BestPathCost = %d\n",bestPathCost);
			if(bestPath != NULL) {
//				printf("** Now setting solution[%d] to %c->%d->%c\n",noVisited,currNode->id,bestPath->weight,bestPath->endpoint->id);
				solution[noVisited] = bestPath->endpoint;
				return bestPathCost;
			}
		}
	}
	return -1;
}


/******* Input data parsing *********/

int parseGraphFile(FILE * file) {
	char cNodeA, cNodeB;
	int weight;
	int n = 0;
	node * ptrA;
	node * ptrB;
	
	while((n = fscanf(file,"%c;%d;%c\n",&cNodeA,&weight,&cNodeB)) == 3) {
		PRINT(DEBUG,"Pr Add: %c -> %d -> %c\n",cNodeA,weight,cNodeB);
		ptrA = getNode(cNodeA);
		ptrB = getNode(cNodeB);
		addEdges(ptrA, ptrB, weight);
	}
	
	return 0;
}


/******* Program behaviour **********/

void printHelp(int status) {
	printf("Here be help some day, maybe\n");
	exit(status);
}

void setVerbosity(int level) {
	if(verbosity == NORMAL)
		verbosity=level;
	else
		EPRINT(ALERT,"Verbosity already set. Ignoring\n");
}

int main(int argc, char* argv[]) {



	bool correctInput=true;
	for(int i=1;i<argc;++i) {
		if(argv[i][0] == '-') {
			int j=1;
			while(argv[i][j] != '\0') {
				switch(argv[i][j++]) {
					case 'h': printHelp(0); //This will exit the program
							  break;
					case 'v': setVerbosity(VERBOSE); //Print more info
							  break;
					case 'd': setVerbosity(DEBUG); //Implies verbose too
							  PRINT(DEBUG,"Setting verbosity to debug\n");
							  break;
					case 's': setVerbosity(SILENT); //Print nothing but critical errors
							  break;
					default:  correctInput = false;
							  EPRINT(CRITICAL,"Unknown parameter -%c\n",argv[i][j]);
							  break;
				}
			}
		}
		else {
			if(first == NULL) {
				EPRINT(VERBOSE,"Trying to read file %s... ",argv[i]);
				graphFile = fopen(argv[i], "r");
				if(graphFile == NULL) {
					EPRINT(VERBOSE, "failed\n");
					EPRINT(CRITICAL, "Failed to open %s: %s\n",argv[i], strerror(errno));
					exit(errno);
				}
				else {
					EPRINT(VERBOSE, "done\n");
					parseGraphFile(graphFile);
					PRINT(DEBUG,"Done parsing graph\n");
					fclose(graphFile);
				}
			}
			else {
				EPRINT(ALERT,"Only one file can be supplied per run.\n Ignoring %s\n",argv[i]);
			}
		}
		if(!correctInput) {
			printHelp(1);
		}
	}
	
	EPRINT(VERBOSE, "Traveling graph from %c:\n", entrypoint->id);
	travelGraph();
/*
	printf("Entrypoint: %c\n", entrypoint->id);

	node * visited[5] = {entrypoint, 0, 0, 0, 0};
	int noVisited = 4;

	bool ret = visit(entrypoint->edges->next->endpoint, visited, &noVisited);
	printf("Visited %c, ret: %d\n", entrypoint->edges->next->endpoint->id, ret);
*/
//	listEdges('A');
//	listEdges('B');

	freeAllNodes(first);
	return 0;
}
