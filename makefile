project: my_testmgr.o chunk.o heapmngr.o
	cc my_testmgr.o chunk.o heapmngr.o -o project
my_testmgr.o: my_testmgr.c heapmngr.h
	cc -Wall -c my_testmgr.c
chunk.o: chunk.c chunk.h
	cc -Wall -c chunk.c
heapmngr.o: heapmngr.c heapmngr.h chunk.h
	cc -Wall -c heapmngr.c
