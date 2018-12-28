/*
*File:mymake2.c
*Author:Jiaming Hao
*Purpose:This program involves extending the mymake program from project09 to have it implement the core functionality of  
*the make utility. Besides constructing and traversing a graph structure reflecting the dependencies specified in an input file, this
*program also check for file existence and last-modified timestamps associated with the files, and executing the command associated with a 
*target when it is necessary ti rebuild the target.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "graph.h"
#include "mymake2.h"
//declare the following two pointers as global in order to let other programs to call my_free to free them
char* line=NULL;
FILE* fp=NULL;
/*
*void trim():mainly erase the leading spaces and trailing spaces in the given string
*/
void trim(char* pStr)
{
	char* p=pStr;
	char* temp=pStr;
	while(*p==' '||*p=='\t')//not only erase the leading white spaces but also erase the leading 0s
	{
		p++;
	}
	while(*p!='\0')
	{
		*pStr=*p;
		pStr++;
		p++;
	}
	*pStr='\0';
	if(pStr>temp)
	{
		pStr=pStr-1;
		while(*pStr==' '||*pStr=='\t')
		{
			*pStr='\0';
			pStr--;
			if(pStr<=temp)
			{
				break;
			}
		}
	}
}
/*
*void my_free():free the heap used by getline function and fopen function(the only function included in the mymake.h)
*/
void my_free()
{
	free(line);
	fclose(fp);
}

int main(int argc, char* argv[])
{
 	char* ptr; //this ptr deal with the things related with line read in
	size_t len=0;
	int length=0;
	char target[66];//might include ":", so 64+1+1=66
	char read[65];
	char file_name[65]="";//store the name of the file used to construct dependency tree
	int status=0;
	char aTarget[65]="";//store the name of the node as the starting point of postorder travesal later
	int command_line=0;//indicate this line is command or not
	//printf("The number of arguments read in is %d\n",argc);//debug
	if(argc>4)//mymake2 accepts max number of arguments is 4
	{
		fprintf(stderr,"Too many arguments\n");
		return 1;
	}
	else if(argc==1)//when there are no arguments provided
	{
		strcpy(file_name,"myMakefile");//the default file name is "myMakefile"
	}
	else if(argc==2)//if there are two arguments altogether, the second argument must be target, otherwise it is an error
	{
		if(strcmp(argv[1],"-f")==0)
		{
			fprintf(stderr,"There must be a file name follows the \"-f\" option\n");
			return 1;
		}
		else
		{
			//printf("Here\n");//debug
			strcpy(aTarget,argv[1]);
			//printf("The content of aTarget is %s\n",aTarget);//debug
		}
	}
	else if(argc==3)
	{
		int has_f=0;//keep track of how many times "-f" appears, it is an error when it is over 1
		if(strcmp(argv[1],"-f")==0)//by default, the argument follows the -f is the name of the makefile
		{
			strcpy(file_name,argv[2]);
			has_f++;
		}
		if(strcmp(argv[2],"-f")==0)
		{
			if(has_f!=0)
			{
				fprintf(stderr,"More than one \"-f\" appears in the arguments\n");
				return 1;
			}
			else
			{
				fprintf(stderr,"There must be a file name follows the \"-f\" option\n");
				return 1;
			}
		}
		if(strcmp(argv[1],"-f")!=0&&strcmp(argv[2],"-f")!=0)
		{
			fprintf(stderr,"illegal arguments provided(more than one targets or file name without a -f preceeding)\n");
			return 1;
		}
		
	}
	else//this is the case when argc==4
	{
		int has_f=0;
		if(strcmp(argv[1],"-f")==0)//by default, the argument follows the -f is the name of the makefile
		{
			strcpy(file_name,argv[2]);
			has_f++;
			strcpy(aTarget,argv[3]);
		}
		if(strcmp(argv[2],"-f")==0)//if argv[2] is "-f", then argv[3] is the file name and argv[1] is the target specified
		{
			if(has_f!=0)
			{
				fprintf(stderr,"More than one \"-f\" appears in the arguments\n");
				return 1;
			}
			strcpy(file_name,argv[3]);
			has_f++;
			strcpy(aTarget,argv[1]);
		}
		if(strcmp(argv[3],"-f")==0)
		{
			if(has_f!=0)
			{
				fprintf(stderr,"More than one \"-f\" appears in the arguments\n");
				return 1;
			}
			else
			{
				fprintf(stderr,"There must be a file name follows the \"-f\" option\n");
				return 1;
			}
		}
		if(strcmp(argv[1],"-f")!=0&&strcmp(argv[2],"-f")!=0&&strcmp(argv[3],"-f")!=0)
		{
			fprintf(stderr,"illegal arguments provided(more than one targets or file name without a -f preceeding)\n");
			return 1;
		}
	}
	if(strcmp(file_name,"")==0)
	{
		strcpy(file_name,"myMakefile");
	}
	//printf("The file name is %s\n",file_name);//debug
	fp=fopen(file_name,"r");
	if(fp==NULL)
	{
		//notice that this part fp can not be freed by calling fclose, otherwieh it will crash
		perror(file_name);
		free(line);
		exit(1);
	}
	//printf("Here\n");//debug
	int count=0;
	while(getline(&line,&len,fp)!=EOF)
	{
		int has_colon=0;//keep track of whether there is already a colon in the current line read in
		length=strlen(line);
		if(command_line==0&&line[0]=='\t')//if the line is the first line and it is a command line, then report error
		{
			fprintf(stderr,"Error in file format, there is a tab at the beginning of first line\n");
			free(line);
			fclose(fp);
			garbage_clean();
			return 1;
		}
		if(line[length-1]=='\n')//first erase the newline character if there is one
		{
			line[length-1]='\0';
		}
		if(line[0]=='\t')//we distinguish a command line from a target line by checking whether the first character of the line is a tab or not
		{
			//printf("The value of command_line is %d\n",command_line);//debug
			//printf("Command line is %s\n",line);//debug
			//printf("Target is %s\n",target);//debug
			//printf("Command line %s belongs to target: %s\n",line,target);//debug
		  	command_line++;
			trim(line);//trim the leading tab and white spaces
			if(strcmp(line,"")!=0)//if the command line after modifaction is totally empty, then ignore it
			{
				add_command(target,line);
			}
				continue;
		}
		//first extract the target from each line
		status=sscanf(line,"%65s",target);
		if(status==-1)//this probably means the line is totally empty, and since it is empty, it is not counted as a target line
		{
			continue;
		}
		//printf("The target before modifaction is %s\n",target);//debug
		int target_len=strlen(target);
		//printf("target length is %d\n",target_len);//debug
		if(target[target_len-1]==':')
		{
			has_colon=1;
			target[target_len-1]='\0';
		}
		//printf("The target after modifaction is %s\n",target);
		command_line++;//as long as the line has a target, then it is considered to be a target line even if there is nothing related with the target
		int i;
		for(i=0;i<target_len;i++)//check except the last element of the target are allowed to be ":", if there is elsewhere in the target has":"
		{
			if(target[i]==':')
			{
				if(has_colon==1)//if a colon has already been found before, then this is the second colon, it is illegal
				{
					fprintf(stderr,"illegal line %s\n",line);
					fclose(fp);
					free(line);
					garbage_clean();
					return 1;//since till now we have not add anything to the graph yet, do not need to call garbage_clean
				}
				else
				{
					has_colon=1;//indicate there is already one colon appeared in the target line, which means if still has colon later, it is illegal
					target[i]='\0';
					break;
				}
			}
		}
		//printf("The target after modifaction is %s\n",target);
		if(strcmp(target,"")==0)
		{
			fprintf(stderr,"No target in the rules\n");
			fclose(fp);
			free(line);
			garbage_clean();
			return 1;
		}
		if(strcmp(aTarget,"")==0&&count==0)//the default target is the first target appeared in the Makefile
		{
			strcpy(aTarget,target);
		}
		addVertex(target,1);//this can include repeated target check
		//at this point, the target of each line has been extracted, then we need to find out vertex related with this target
		count++;
                i=0;//use i again
		for(ptr=line;*ptr!='\0';ptr++)
		{
			if(isspace(*ptr))//this is important, because of this statement, when get out of the loop, ptr must point to something other than whitespace!
			{
				continue;
			}
			else
			{
				if(target[i]=='\0')
				{
					break;
				}
				else
				{
					i++;
				}
			}
		}
		//printf("ptr points to %c\n",*ptr);//debug
		if(*ptr==':')//this can only happen when there is no space before ":",which means the target and the colon are combined together
		{
			if(has_colon==0)
			{
				has_colon=1;
			}
			//printf("Still is colon\n");//debug
			ptr++;
		}
		if(*ptr==':')//this follow the above situation, when something like "target::" or "target ::", it should be considered as illegal
		{	
			if(has_colon==1)
			{
				fprintf(stderr,"illegal line %s\n",line);
				fclose(fp);
				free(line);
				garbage_clean();
				return 1;
			}
			//else
			//{
				//has_colon=1;
			//}
		}
		if(has_colon==0)//if until this step it still does not include a colon, this line is illegal because ther are no colon folllowing target
		{
			fprintf(stderr,"illegal line %s\n",line);
			fclose(fp);
			free(line);
			garbage_clean();
			return 1;
		}
		status=sscanf(ptr,"%64s",read);
		if(status==-1)//this probably means the line only has a target or a target with colon?
		{
			//command_line++;
			continue;
		}
		for(i=0;i<strlen(read);i++)//a word is not allowed to contain ":"
		{
			if(read[i]==':')
			{
				fprintf(stderr,"illegal line %s\n",line);
				fclose(fp);
				free(line);
				garbage_clean();
				return 1;
			}
		}
		//printf("First realted one is %s\n",read);//debug
		addVertex(read,0);
		addEdge(target,read);
		//printf("ptr before loop is %c\n",*ptr);
		while(*ptr!='\0')
		{
			i=0;
			for(;ptr!='\0';ptr++)
			{
				if(isspace(*ptr))//this is important, because of this statement, when get out of the loop, ptr must point to something other than whitespace!
				{
					continue;
				}
				else
				{
					if(read[i]=='\0')
					{
						break;
					}
					else
					{
						i++;
					}
				}
			}
			status=sscanf(ptr,"%64s",read);
			for(i=0;i<strlen(read);i++)
			{
				if(read[i]==':')
				{
					fprintf(stderr,"illegal line %s\n",line);
					fclose(fp);
					free(line);
					garbage_clean();
					return 1;
				}
			}
			//printf("ptr is %c\n",*ptr);//debug
			if(status!=-1)//this probably means there are no more vertexs related to be read
			{
				//printf("Next related is %s\n",read);//debug
				addVertex(read,0);
				addEdge(target,read);
			}
			else
			{
				break;
			}
		}
	}
	print_in_postorder(aTarget);	
	free(line);
	garbage_clean();
	fclose(fp);
	if(error!=0)
	{
		return 1;
	}
	return 0;
}
