mymake2: mymake2.o graph.o
	gcc -Wall -g -o mymake2 mymake2.o graph.o -L/home/cs352/spring17/Assignments/proj09 -lcs352

mymake2.o: mymake2.c graph.h mymake2.h
	gcc -Wall -g -c mymake2.c

graph.o: graph.c  graph.h mymake2.h
	gcc -Wall -g -c graph.c 

.PHONY: clean
clean:
	rm -f *.o mymake2


