CC=gcc
LIB_DIR=../lib

all: testcFila testcThread testcSemaforo

testcFila: testcFila.c $(LIB_DIR)/libcthread.a
	$(CC) -o testcFila testcFila.c  -L$(LIB_DIR) -lcthread -Wall
	
testcThread: testcThread.c $(LIB_DIR)/libcthread.a
	$(CC) -o testcThread testcThread.c -L$(LIB_DIR) -lcthread -Wall
	
testcSemaforo: testcSemaforo.c $(LIB_DIR)/libcthread.a
	$(CC) -o testcSemaforo testcSemaforo.c -L$(LIB_DIR) -lcthread -Wall

clean:
	rm -rf testcFila testcThread *.o *~
