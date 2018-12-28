/*
*File:graph.h
*Author:Jiaming Hao
*Purpose:Providing the method used to construct dependency tree and travesal the tree in 
*postorder.
*/
#ifndef GRAPH_H
#define GRAPH_H
void addVertex(char* input,int par);
void addEdge(char* from,char* dest);
void add_command(char* target,char* command);
void print_in_postorder(char* target);
void garbage_clean();
extern int error;

#endif
