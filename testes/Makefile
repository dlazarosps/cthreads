CC=gcc
LIB_DIR=../lib

all: testcFila testcThread

testcFila: testcFila.c $(LIB_DIR)/libcthread.a
	$(CC) -o testcFila testcFila.c -L$(LIB_DIR) -lcthread -Wall
	
testcThread: testcThread.c $(LIB_DIR)/libcthread.a
	$(CC) -o testcThread testcThread.c -L$(LIB_DIR) -lcthread -Wall

clean:
	rm -rf testcFila testcThread *.o *~
