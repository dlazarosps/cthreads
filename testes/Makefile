CC=gcc
LIB_DIR=../lib

all: testcFila testCidentify

testcFila: testcFila.c $(LIB_DIR)/libcthread.a
	$(CC) -o testcFila testcFila.c -L$(LIB_DIR) -lcthread -Wall
	
testCidentify: testCidentify.c $(LIB_DIR)/libcthread.a
	$(CC) -o testCidentify testCidentify.c -L$(LIB_DIR) -lcthread -Wall

clean:
	rm -rf testcFila testCidentify *.o *~
