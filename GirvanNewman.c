// Girvan-Newman Algorithm for community discovery
// COMP2521 Assignment 2

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "CentralityMeasures.h"
#include "GirvanNewman.h"
#include "Graph.h"

static void dfsComponent(int **g, int size, int *componentOf, int componentID, int vertex);
static Dendrogram insertDendrogram(Dendrogram dg, int *componentOf, int vertex, int depth, int helpCount);
//static bool isOnlyElementOfComponent(int vertex, int size, int *componentOf);

/**
 * Generates  a Dendrogram for the given graph g using the Girvan-Newman
 * algorithm.
 * 
 * The function returns a 'Dendrogram' structure.
 */
Dendrogram GirvanNewman(Graph g) {
	
	int n = GraphNumVertices(g);
	EdgeValues ev = edgeBetweennessCentrality(g);
	
	Dendrogram dg = malloc(sizeof(DNode));
	//Dendrogram current_dg_node = dg;
	 dg->vertex = -1; // -1 for non-leave node
	 dg->left = NULL;
	 dg->right =NULL;
	
	int original_num_components = 1;
	int depth = 0; // current depth, start from 0, +1 at each iteration
	
	// adjacency matrix form of the graph
	int **graphCopy = malloc(n * sizeof(int *));
	for(int i = 0; i < n; i++){
	    graphCopy[i] = malloc(n * sizeof(int));
		for(int j = 0; j < n; j++)
			graphCopy[i][j] = -1;	// default all to -1
	}
	
	// get the graph edges 
	AdjList l = NULL;
	for(int i = 0; i < n; i++){
		l = GraphOutIncident(g, i); // vertices start from 0 to n-1
		while(l){
			graphCopy[i][l->v] = l->weight;
			printf("%d -> %d = %d \n", i, l->v, l->weight);
			l = l->next;	
		}
	}
	
	// start loop below
	ev = edgeBetweennessCentrality(g);
	int max = 0;	
	// find the edge with highest betweenness 
	for(int i = 0; i < n; i++){
		for(int j = 0; j < n; j++){
			if(ev.values[i][j] > max)
				max = ev.values[i][j];
		}
	}
	printf("highest betweenness: %d\n", max);
	
	
	//debug, print the memo table out
	for(int i = 0; i < n; i++){
		for(int j = 0; j < n; j++){
			printf("%10d ", graphCopy[i][j]);
		}
		putchar('\n');
	}
	
	// delete edges with highest betweenness
	for(int i = 0; i < n; i++){
		for(int j = 0; j < n; j++){
			if(ev.values[i][j] == max){
				graphCopy[i][j] = -1;
				ev.values[i][j] = -1;
				GraphRemoveEdge(g, i, j);
			}	
		}
	}
	putchar('\n');
	
	// to make it undirected graph, since we just want to see if weakly connected, don't care strongly connectedness.
	for(int i = 0; i < n; i++){
		for(int j = 0; j < n; j++){
			if(graphCopy[i][j]){
				graphCopy[j][i] = graphCopy[i][j]; // for those left undeleted, make undirected for DFS
			}	
		}
	}
	
	//debug, print the memo table out
	for(int i = 0; i < n; i++){
		for(int j = 0; j < n; j++){
			printf("%10d ", graphCopy[i][j]);
		}
		putchar('\n');
	}
	
	// mark the components after removing highest between vertices
	// record which component does each vertex belong to 
	int componentID = 1;
	int *componentOf = malloc(n * sizeof(int));
	for(int i = 0; i < n; i++)
		componentOf[i] = -1;
	
	
	// apply dfsComponent to every vertex of the graph (mark a componentID)
	for(int i = 0; i < n; i++){
		if(componentOf[i] == -1){
			dfsComponent(graphCopy, n, componentOf, componentID, i);
			componentID++;
		}
	}
	
	// new component has been formed
	if(componentID > original_num_components){
		original_num_components = componentID;
		depth += 1;
	}
	
	// debug, show the components of each vertex
	for(int i = 0; i < n; i++){
		printf("%d: component %d \n", i, componentOf[i]);
	}
	
	// int helperCount = 0; // depth starts from 0(first parent node)
	// // add the vertices into subtrees in groups of their componentID
	// for(int i = 0; i < n; i++){
	// 	if(isOnlyElementOfComponent(i, componentOf)){ // only add if the vertex is only member of component
	// 		insertDendrogram(dg, componentOf, i, depth, helperCount);
	// 		helperCount = 0; // reset after every insert
	// 	}
	// }
	int helperC = 0;
	dg = insertDendrogram(dg, componentOf, 5, 1, helperC);
	helperC = 0;
	dg = insertDendrogram(dg, componentOf, 3, 2, helperC);
	helperC = 0;
	dg = insertDendrogram(dg, componentOf, 9, 3, helperC);
	helperC = 0;
	dg = insertDendrogram(dg, componentOf, 6, 3, helperC);
	
	//
	// dg->vertex = 9;
	// dg->left = malloc(sizeof(DNode));
	// dg->left->vertex = 4;
	// dg->left->left = NULL;
	// dg->left->right = NULL;
	//
	// dg->right = malloc(sizeof(DNode));
	// dg->right->vertex = 3;
	// dg->right->left = NULL;
	// dg->right->right = NULL;

	return dg;
}

/**
 * Frees all memory associated with the given Dendrogram  structure.  We
 * will call this function during testing, so you must implement it.
 */
void freeDendrogram(Dendrogram d) {
	// TODO: Implement this function
}

static void dfsComponent(int **g, int size, int *componentOf, int componentID, int vertex){
	componentOf[vertex] = componentID;
	
	for(int k = 0; k < size; k++){
		if(g[vertex][k] != -1 && componentOf[k] == -1){
			dfsComponent(g, size, componentOf, componentID, k);
		}
	}
}

// depth:destination dendrogram insertion depth
static Dendrogram insertDendrogram(Dendrogram dg, int *componentOf, int vertex, int depth, int helperCount){
	printf("\ntrying insert %d to depth %d, helperCount%d\n", vertex, depth, helperCount);
	if(dg == NULL){
		Dendrogram newNode = malloc(sizeof(DNode));
		newNode->left = NULL;
		newNode->right = NULL;
		
		if(helperCount == depth){
			printf("created node %d\n", vertex);
			newNode->vertex = vertex; // leaf
		}
		else if(helperCount < depth){
			newNode->vertex = -1;	// rootstock
			printf("created rootstock\n");
			// when node insertion requires creating a new rootstock, default add the node to left of new rootstock
			Dendrogram dgLeft = malloc(sizeof(DNode));
			dgLeft->right = NULL;
			dgLeft->left = NULL;
			dgLeft->vertex = vertex;
			
			newNode->left = dgLeft;
		}
		else{
			printf("NO WAY MAN!!!\n");
		}
		
		printf("generated node %d at depth %d, target depth:%d\n", vertex, helperCount, depth);
		return newNode;
	}
	printf("got here, dg verterx:%d\n", dg->vertex);
	
	// proceed to insert if it's a rootstock
	if(dg->vertex == -1){
		if(dg->left == NULL){ // if left empty
			printf("left empty, proceed\n");
			dg->left = insertDendrogram(dg->left, componentOf, vertex, depth, helperCount+1);
		}
		else if(dg->left && dg->left->vertex == -1){ // if left is a rootstock
			printf("left is rootstock, proceed\n");
			dg->left = insertDendrogram(dg->left, componentOf, vertex, depth, helperCount+1);
		}
		else if(dg->right == NULL){ // if right empty 
			printf("right is NULL, proceed\n");
			dg->right = insertDendrogram(dg->right, componentOf, vertex, depth, helperCount+1);
		}
		else if(dg->right && dg->right->vertex == -1){ // if right is a rootstock
			printf("right is rootstock, proceed\n");
			dg->right = insertDendrogram(dg->right, componentOf, vertex, depth, helperCount+1);
		}
		else{
			printf("THIS SHOULDN't HAPPEN!!!\n");
		}
	}
	else{
		printf("what heppened?\n");
	}
	
	return dg;
	// todo: consider componentOf and depth, and added already?
}

// static bool isOnlyElementOfComponent(int vertex, int size, int *componentOf){
// 	int n = 0;
// 	int componentID = componentOf[vertex];
//
// 	for(int i = 0; i < size; i++){
// 		if(componentOf[i] == componentID){
// 			n++;
// 		}
// 		if(n > 1)
// 			return false;
// 	}
//
// 	return true;
// }
