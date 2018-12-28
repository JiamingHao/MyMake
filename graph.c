/*
*File:graph.c
*Author:Jiaming Hao
*Purpose:Do not contain main method. All of the methods used by mymake.c to build a tree
*are defined here. Most of the methods are included in the header file graph.h.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include "graph.h"
#include  "/home/cs352/spring17/Assignments/proj09/cs352.h" 
#include "mymake2.h"

int error=0;
int track=0;//this variable keeps track of whether there are commands executed during each runtime
/*
*Structure of the most outside node, each word read in will be treated as a vertex
*/
struct vertex{
	char name[65];//record the name of each vertex
	struct command_list* command_head;//if the vertex is a target,then this pointer using strdup to store its command,notice that a target can have multiple commands
	struct edge_list* list_head;
	int visit;//keep track of whether the vertex has been visited or not,this is something a little different than cycle
	int complete;//indicating whether the current target has been completed or not
	struct vertex* next_vertex;
	int target_or_not;// 0 means not a target, 1 means a target
	long last_modi_time; //timestamp
	long last_modi_time_sec;//timestamp
	long last_modi_time_nsec;//timestamp
};
/*
*Structure recording the edges of each vertex
*/
struct edge_list{
	struct vertex* edge;
	struct edge_list* next_edge;
};
/*
*Structure recording the commands of each target, notice a target can have many commands
*/
struct command_list{
	char* command;
	struct command_list* next_command;
};
struct vertex* start_point=NULL;

/*
*void addVertex(char* input, int par):creating a new vertex in the tree, par indicating the node is a target or just an element of dependency
*/
void addVertex(char* input,int par)//the input here is already after checking,the par indicates whether this vertex is added as target(1)or just as dependency(0)
{
	struct vertex* ptr = start_point;
	if(ptr==NULL)//this is the case that the newly added vertex is the first vertex of the graph
	{
		struct vertex* newVertex=cs352_malloc(sizeof(struct vertex));
		if(newVertex==NULL)
		{
			//fprintf(stderr,"ERROR:Out of memory in method addVertex when creating new node as the first node\n");
			fprintf(stderr,"ERROR: 1st malloc failed\n");
			my_free();//a method imported from mymake.c
			garbage_clean();
			exit(1);//do not forget to clean up the heap memory before exit	
		}
		strcpy(newVertex->name,input);
		newVertex->list_head=NULL;
		newVertex->visit=0;//0 represents that this vertex has not been visited so far
		newVertex->complete=0;//0 represents that this vertex has not been completed
		newVertex->last_modi_time=0;
		newVertex->last_modi_time_sec=0;
		newVertex->last_modi_time_nsec=0;
		newVertex->next_vertex=NULL;
		newVertex->command_head=NULL;//command will be added later even the vertex added is a target
		if(par==1)
		{
			newVertex->target_or_not=1;
		}
		else
		{
			newVertex->target_or_not=0;
		}
		start_point=newVertex;
		//printf("The first vertex %s has been added to the graph\n",input);//debug
		return;
	}
	else//if there are already vertexs existing in the graph, then add the new vertex to the end of the "out list"
	{
		while(ptr->next_vertex!=NULL&&strcmp(ptr->name,input)!=0)
		{
			ptr=ptr->next_vertex;
		}
		if(ptr->next_vertex!=NULL)//this is the case that there is already a vertex with the same name exsiting in the graph, we just need to return
		{
			//printf("Vertex %s already exists\n and its target value is %d\n",input,ptr->target_or_not);//debug
			if(par==1&&ptr->target_or_not==1)//this is the case that the vertex already exists in the graph, and it is already a target, in this case, it is an error
			{
				fprintf(stderr,"ERROR: repeated target %s\n",input);
				my_free();
			    	garbage_clean();
				exit(1);
			}
			else if(par==1&&ptr->target_or_not==0)//if the vertex already exists but has not been marked as target, update it to be a target, and it is not a error
			{
				ptr->target_or_not=1;
			}
			//there are stll two situation reminded, when par==0, indicating the current vertex is added as a dependency, both of them will not cause error
			return;
		}
		else//this is the case when ptr->next==NULL,but be careful that at this time the name of the current vertex has not been checked yet!
		{
			if(strcmp(ptr->name,input)==0)
			{
				//printf("Vertex %s already exists\n",input);//debug
				if(par==1&&ptr->target_or_not==1)//same as above
				{
					fprintf(stderr,"ERROR: repeated target %s\n",input);
					my_free();
			        	garbage_clean();
					exit(1);
				}
				else if(par==1&&ptr->target_or_not==0)
				{
					ptr->target_or_not=1;
				}
				return;	
			}
			struct vertex* newVertex=cs352_malloc(sizeof(struct vertex));
			if(newVertex==NULL)
			{
				//fprintf(stderr,"ERROR:Out of memory in method addVertex when creating new vertex\n");
				fprintf(stderr,"ERROR: Second malloc failed\n");
				my_free();
			    	garbage_clean();
				exit(1);	
			}
			strcpy(newVertex->name,input);
			newVertex->list_head=NULL;
			newVertex->visit=0;//0 represents that this vertex has not been visited so far
			newVertex->complete=0;//0 represents that this vertex has not been completed so far
			newVertex->last_modi_time=0;
			newVertex->last_modi_time_sec=0;
			newVertex->last_modi_time_nsec=0;
			newVertex->next_vertex=NULL;
			newVertex->command_head=NULL;//command will be added later even if the vertex added now is a target
			if(par==1)
			{
				newVertex->target_or_not=1;
			}
			else
			{
				newVertex->target_or_not=0;
			}
			ptr->next_vertex=newVertex;
			//printf("New vertex %s has been added to the graph\n",input);
			return;
		}
	}
}
/*
*void addEdge(char* from,char* dest):adding edge from one vertex to the other
*/
void addEdge(char* from,char* dest)//notice that the node who has been added edge to some other node becomes target
{//and also notice  because of the organization, when we call addEdge, ther must already a from and dest existing in the graph
	struct vertex* ptr1=start_point;
	struct vertex* ptr2=start_point;
	/*if(start_point==NULL)//i wonder if this check is needed in mymake, because we first must add an target, then do something
	{
		fprintf(stderr,"Error: There is currently no vertexs in the graph\n");
		//error++;
		return;	
	}*/
	while(ptr1->next_vertex!=NULL&&strcmp(ptr1->name,from)!=0)
	{
		ptr1=ptr1->next_vertex;
	}
	/*if(ptr1->next_vertex==NULL&&strcmp(ptr1->name,from)!=0)
	{
		fprintf(stderr,"Error: No departing vertex \"%s\" currently exists in the graph\n",from);
		//error++;
		return;
	}//if the method does not return here, it means that the starting point has been found in the graph*/
	while(ptr2->next_vertex!=NULL&&strcmp(ptr2->name,dest)!=0)
	{
		ptr2=ptr2->next_vertex;
	}
	/*if(ptr2->next_vertex==NULL&&strcmp(ptr2->name,dest)!=0)
	{
		fprintf(stderr,"Error: No destination vertex \"%s\" currently exists in the graph\n",dest);
		//error++;
		return;
	}//if the method does not return here, it means the dest point has been found in the graph*/
	ptr1->target_or_not=1;//as soon as the node has dependency relation, it becomes a target
	if(ptr1->list_head==NULL)//this is the case that it is the first edge coming from this vertex(target)
	{
		struct edge_list* newEdge = cs352_malloc(sizeof(struct edge_list));
		if(newEdge==NULL)
		{
			//fprintf(stderr,"Out of mempry when creating new edges\n");
			fprintf(stderr,"ERROR: Third malloc failed\n");
			my_free();
			garbage_clean();
			exit(1);
		}
		newEdge->edge=ptr2;
		newEdge->next_edge=NULL;
		ptr1->list_head=newEdge;
		//printf("new edge has been added as the first edge of %s vertex\n",ptr1->name);//debug
		return;
	}
	else//this is the case when list_head!=NULL, which means this vertex(target) already has some edges
	{
		struct edge_list* ptr3=ptr1->list_head;
		while(ptr3->next_edge!=NULL&&ptr3->edge!=ptr2)
		{
			ptr3=ptr3->next_edge;
		}
		if(ptr3->next_edge==NULL&&ptr3->edge!=ptr2)//this is the case that this is a new edge 
		{
			struct edge_list* newEdge = cs352_malloc(sizeof(struct edge_list));
			if(newEdge==NULL)
			{
				//fprintf(stderr,"Out of mempry when creating new edges\n");
				fprintf(stderr,"ERROR: Fourth malloc failed\n");
				my_free();
			   	garbage_clean();
				exit(1);
			}
			newEdge->edge=ptr2;
			newEdge->next_edge=NULL;
			ptr3->next_edge=newEdge;
			//printf("new edge has been added to \"%s\" vertex\n",ptr1->name);//debug
			return;
		}
		else if(ptr3->next_edge==NULL&&ptr3->edge==ptr2)//edge already exists and it is the last edge of the current vertex 
		{
			//fprintf(stderr,"Error [addEdge]: Edge already exists: the last edge coming from \"%s\" vertex\n",ptr1->name);//debug
			//error++;
			return;
		}
		else
		{
			//fprintf(stderr,"Error [addEdge]: Edge already exists\n");//debug
			//error++;
			return;
		}
	}
}
/*
*void add_command(char* target, char* command):add command to one target
*/
void add_command(char* target, char* command)//notice that one target can have multiple commands
{
	struct vertex* ptr1=start_point;
	while(ptr1->next_vertex!=NULL&&strcmp(ptr1->name,target)!=0)//first of all,find the target
	{
		ptr1=ptr1->next_vertex;
	}
	ptr1->target_or_not=1;
	if(ptr1->command_head==NULL)//currently no command of this target
	{
		struct command_list* newCommand=cs352_malloc(sizeof(struct command_list));
		if (newCommand==NULL)
		{
			fprintf(stderr,"ERROR: Fifth malloc failed\n");
			my_free();
			garbage_clean();
			exit(1);
		}
		else
		{
			newCommand->command=cs352_strdup(command);
			if(newCommand->command==NULL)
			{
				fprintf(stderr,"ERROR:First strdup failed\n");
				free(newCommand);///////////
				my_free();
				garbage_clean();
				exit(1);
			}
			newCommand->next_command=NULL;
			ptr1->command_head=newCommand;
		}
	}
	else//already has command
	{
		struct command_list* ptr2=ptr1->command_head;
		while(ptr2->next_command!=NULL)
		{
			ptr2=ptr2->next_command;
		}
		struct command_list* newCommand=cs352_malloc(sizeof(struct command_list));
		if (newCommand==NULL)
		{
			fprintf(stderr,"ERROR: Sixth malloc failed\n");
			my_free();
			garbage_clean();
			exit(1);
		}
		else
		{
			newCommand->command=cs352_strdup(command);
			if(newCommand->command==NULL)
			{
				fprintf(stderr,"ERROR:Second strdup failed\n");
				my_free();
				garbage_clean();
				exit(1);
			}
			newCommand->next_command=NULL;
			ptr2->next_command=newCommand;
		}
	}

	return;
}
/*
*void postOrder(struct vertex* prev,struct vertex* start):support method, working with other methods together to perform postorder travesal
*of the tree,check the timestamp and recompile the target if needed.
*/
int postOrder(struct vertex* prev, struct vertex* start)
{
	//printf("##Call the postOrder method with start points to file: %s\n",start->name);//debug
	struct vertex* temp=prev;
	if(start->visit==1)//if a file has already been visited, then there are two cases, the first case is that this file is contained in a cycle, the second case is that
	//the file is a dependency of multiple targets.(there two cases need to be distinguished)
	{
		if(start->complete==0)//this detects if there is a cycle
		{
			fprintf(stderr,"Circular dropped: there is a cycle between file: \"%s\" and file: \"%s\"\n",start->name,temp->name);//just indicating the cycle but not increase the error
			return 0;
		}
		if((start->last_modi_time)>(prev->last_modi_time))//if the current has already visited and has newer timestamp, then the target above it needs to be recompiled.
		{
			//printf("##Return 1 because file: %s has newer timestamp than file: %s\n",start->name,prev->name);//debug
			return 1;//notice that we just stop at the file that has been visted instead of going deep to check its dependency files(because if it has already been visited before
			//the first time it was visted, it should already be recompiled with the newest timestamp
		}
		else if(start->last_modi_time==prev->last_modi_time)
		{
			if((start->last_modi_time_sec)>(prev->last_modi_time_sec))
			{
				//printf("##Return 1 because file: %s has the newer timestamp than file: %s in SECONDS\n",start->name,prev->name);//debug
				return 1;
			}
			else if(start->last_modi_time_nsec > prev->last_modi_time_nsec)
			{
				//printf("##Return 1 because nanosecond\n");//debug
				return 1;
			}
			else
			{
				//printf("##After comparing the timestamps in seconds, return 0\n");//debug
				return 0;
			}
		}
		else
		{
			//printf("##Return 0\n");//debug
			return 0;
		}		
	}
	else//if the file has not been visited yet
	{
		//printf("##file: %s has not been visited yet\n",start->name);//debug
		start->visit=1;//mark the file as visited
		struct stat buf;
		int status;
		status=stat(start->name,&buf);
		if(status==-1&&start->target_or_not==0)//if the target file does not exist and it is not a target
		{
			fprintf(stderr,"No rules given to build file: %s\n",start->name);
			my_free();
			garbage_clean();
			exit(1);
		}
		else if(status==-1&&start->target_or_not==1)//if the target file does not exist and is a target.
		{
			start->last_modi_time=0;
		}
		else//if the target exists, grab its timestamp
		{
			start->last_modi_time=(long)buf.st_mtime;
			start->last_modi_time_sec=(long)buf.st_mtim.tv_sec;
			start->last_modi_time_nsec=(long)buf.st_mtim.tv_nsec;
			//printf("##Before executing commands, file: %s last_modi_time: %ld, last_modi_time_sec: %ld, last_modi_time_nsec: %ld\n",start->name,start->last_modi_time,start->last_modi_time_sec,start->last_modi_time_nsec);//debug
		}
	}
	struct edge_list* ptr=start->list_head;
	temp=start;
	int recompile_or_not=0;//indicationg whether the start file need to be recompiled or not, 0 means not, 1 means recompile
	int re=0;//store the return value of each recursively call of method postOrder
	while(ptr!=NULL)//if the target has dependencies, itreate through its dependencies
	{
		re=postOrder(temp,ptr->edge);//recursively call
		if(re==1)
		{
			recompile_or_not=re;
		}
		ptr=ptr->next_edge;
	}
	//printf("##file: %s reacompile_or_not: %d\n",start->name,recompile_or_not);//debug
	if(start->last_modi_time==0||recompile_or_not==1)//if the file itself does not exist or it exists but needs to be recompiled
	{
		//printf("Before executing file: %s commands, the last modify time is %ld\n",start->name,start->last_modi_time);//debug
		struct command_list* cptr=start->command_head;
		if(cptr!=NULL)//if the target needed to be recompiled has at least one command
		{
			track++;
			while(cptr->next_command!=NULL)
			{
				printf("%s\n",cptr->command);//print the command just before it being executed
				int result=system(cptr->command);
				if(result==0)
				{
					//printf("The command  %s of file %s has been successfully executed\n",cptr->command,start->name);//debug
				}
				else
				{
					fprintf(stderr,"The execution of command \"%s\" of file \"%s\" failed\n",cptr->command,start->name);
					my_free();
					garbage_clean();
					exit(1);
				}
				cptr=cptr->next_command;
			}
			printf("%s\n",cptr->command);
			int result=system(cptr->command);
			if(result==0)
			{
				//printf("The command %s of file %s has been successfully executed\n",cptr->command,start->name);//debug
			}
			else
			{
				fprintf(stderr,"The execution of command \"%s\" of file \"%s\" failed\n",cptr->command,start->name);
				my_free();
				garbage_clean();
				exit(1);
			}
		}
		struct stat buf;
		int status;
		status=stat(start->name,&buf);
		if(status==-1)//if after the execution of commands, the target file still does not exist
		{
			start->last_modi_time=0;
		}
		else//grab the timestamps of target file again
		{
			start->last_modi_time=(long)buf.st_mtime;
			start->last_modi_time_sec=(long)buf.st_mtim.tv_sec;
			start->last_modi_time_nsec=(long)buf.st_mtim.tv_nsec;
		}
	
		//printf("##After execution, the last modify time of file: %s is %ld\n",start->name,start->last_modi_time);//debug
	}
	start->complete=1;//mark the file as completed
	if(prev!=NULL&&(start->last_modi_time)>(prev->last_modi_time))
	{
		//printf("##Return 1 since file: %s has newer timestamp than file: %s\n",start->name,prev->name);//debug
		return 1;
	}
	else if(prev!=NULL&&(start->last_modi_time)==(prev->last_modi_time))
	{
		if((start->last_modi_time_sec)>(prev->last_modi_time_sec))
		{
			//printf("##Return 1 since file: %s has newer timesamp than file: %s in SECONDS\n",start->name,prev->name);//debug
			return 1;
		}
		else if(start->last_modi_time_nsec > prev->last_modi_time_nsec)
		{
			//printf("##Return 1 because nanoseconds\n");
			return 1;
		}
		else
		{
			//printf("##After comparing the timestamps in seconds, return 0\n");//debug
			return 0;
		}
	
	}
	else if (start->last_modi_time==0)
	{
		//printf("##Return 1 since file: %s does not exist\n",start->name);//debug
		return 1;
	}
	else
	{
		//printf("##Return 0\n");//debug
		return 0;
	}
}	
/*
*void print_in_postorder(char* target): if the mymake.c want to visit a tree starting from a specified target in postorder, then it just call
*this method, and this method calls other supporting method, but what this method actually do is finding the target in the tree and 
*hand it to other methods.
*/
void print_in_postorder(char* target)
{
	struct vertex* ptr1=start_point;
	if(start_point==NULL)
	{
		fprintf(stderr,"Error: There is currently no vertexs in the graph\n");
		error++;
		return;	
	}
	while(ptr1->next_vertex!=NULL&&strcmp(ptr1->name,target)!=0)
	{
		ptr1=ptr1->next_vertex;
	}
	if(ptr1->next_vertex==NULL&&strcmp(ptr1->name,target)!=0)
	{
		fprintf(stderr,"Error: No target vertex \"%s\" currently exists in the graph\n",target);
		error++;
		my_free();
		garbage_clean();
		exit(1);
	}
	if(ptr1->target_or_not==0)
	{
		fprintf(stderr,"Error: %s exists but not a target\n",target);
		my_free();
		garbage_clean();
		exit(1);
	}//if the method does not exit here, means the target has  been found
	postOrder(NULL,ptr1);
	if(track==0)
	{
		printf("%s is up to date.\n",target);
	}
	return;
}
/*
*void garbage_clen():free the tree
*/
void garbage_clean()
{
	struct vertex* ptr1=start_point;
	struct edge_list* ptr2;
	struct command_list* ptr3;
	while(ptr1==NULL)//when there is actually nothing in the graph, the garbage is unneccessary
	{
		return;
	}
	while(ptr1!=NULL)
	{
		ptr2=ptr1->list_head;
		while(ptr2!=NULL)
		{
			struct edge_list* copy=ptr2->next_edge;
			free(ptr2);
			ptr2=copy;
		}
		ptr3=ptr1->command_head;
		while(ptr3!=NULL)
		{
			struct command_list* COPY=ptr3->next_command;
			free(ptr3->command);
			free(ptr3);
			ptr3=COPY;
		}
		struct vertex* Copy=ptr1->next_vertex;
		free(ptr1);
		ptr1=Copy;
	}
}

