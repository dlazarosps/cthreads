#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "cfila.h"
#include "cdata.h"
#include "cthread.h"
#include "support.h"

int cinit(void) {
  int check;

  /*
    Initiate control block elements
  */
  controlBlock.initiated = TRUE;
  check = initFILA2(&controlBlock.allThreads, FALSE);
  if(!check) return -1;

  check = initFILA2(controlBlock.aptoThreads, TRUE);
  if(!check) return -2;

  check = initFILA2(controlBlock.blockedThreads, TRUE);
  if(!check) return -3;

  
  /*
    Creates the main thread.
    Appends it to the allThreads structure.
  */
  TCB_t *mainThread;

  mainThread = (TCB_t*) malloc(sizeof(TCB_t));
  mainThread->tid = 0;
  mainThread->state = PROCST_EXEC;
  mainThread->prio = 0;

  insertFILA2(&controlBlock.allThreads, (void *) mainThread);

  /*
    Set ending function for all created threads
  */
  getcontext(&controlBlock.endThread);
  controlBlock.endThread.uc_link = NULL;
  controlBlock.endThread.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
  controlBlock.endThread.uc_stack.ss_size = SIGSTKSZ;
  makecontext(&controlBlock.endThread, (void (*)(void))endThread, 0);

  /*
    Create context to main thread
    Set Main thread as running
  */
  getcontext(&mainThread->context);
  mainThread->context.uc_link =  &controlBlock.endThread;
  mainThread->context.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
  mainThread->context.uc_stack.ss_size = SIGSTKSZ;

  controlBlock.runningThread = mainThread;

  return 0;
};

void endThread(void){
	getcontext(&controlBlock.endThread);

  // Delete thread from blocking threads

	controlBlock.runningThread->state = PROCST_TERMINO;
	#if DEBUG
  	printf("TID: %i has ended. \n", controlBlock.runningThread->tid);
	#endif

    /* TO DO */
    // verificar threads bloqueadas que estavam esperando thread que terminou

	scheduler();
}

void insertThreadToFila(int prio, void * thread) {
  int finished = FALSE;
  TCB_t* node;
  TCB_t* nextnode;

  PFILA2 fila = controlBlock.aptoThreads;
  FirstFila2(fila);

  do {
    node = (TCB_t*) GetAtIteratorFila2(fila);
    if (node == NULL) //fila vazia
    {
      insertFILA2(fila, (void*) thread);
      finished = TRUE;
    }
    else{
      if(node->prio <= prio)
      {
        nextnode = (TCB_t*) GetAtNextIteratorFila2(fila);
        if(nextnode->prio > prio){
          insertBeforeFILA2(fila, (void*) thread);
          finished = TRUE;
        }
      }
      else{
        insertBeforeFILA2(fila, (void*) thread);
        finished = TRUE;
      }
    }

  } while(!finished);
}


int generateTID(void) {
  /*TO DO */
  //verificar a unicidade dos TIDs criados
	static int globalTID = 0;

	return ++globalTID;
}

int scheduler(void) {
  TCB_t* nextRunningThread;

  if (FirstFila2((PFILA2) &controlBlock.aptoThreads) == 0) {
    nextRunningThread = (TCB_t*) GetAtIteratorFila2((PFILA2) &controlBlock.aptoThreads);
    removeFILA2((PFILA2) &controlBlock.aptoThreads, nextRunningThread->tid);
  } else {
    return -1;
  }

  nextRunningThread->state = PROCST_EXEC;
  dispatcher(nextRunningThread);
  return 0;
}

int dispatcher(TCB_t* nextRunningThread){
  TCB_t* currentThread = controlBlock.runningThread;
  currentThread->state = PROCST_APTO;

  /*TO DO*/
  //falta validar quando for a primeira vez que está rodando  
  currentThread->prio = currentThread->prio + stopTimer(); 
  insertThreadToFila(currentThread->prio, (void *) currentThread);

  controlBlock.runningThread = nextRunningThread;

  swapcontext(&currentThread->context, &nextRunningThread->context);
  startTimer();
	return 0;
}
