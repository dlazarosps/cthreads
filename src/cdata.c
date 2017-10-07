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

	if (searchFILA2join(controlBlock.blockedThreads, controlBlock.runningThread->tid, TRUE) == TRUE)
	{	
		Pjoin* joinThread;
		joinThread = (Pjoin*) GetAtIteratorFila2(controlBlock.blockedThreads);

		TCB_t* releaseThread = joinThread->waiting;
		releaseThread->state = PROCST_APTO;
		insertThreadToFila(releaseThread->prio, (void *) releaseThread);
	}

	scheduler();
}

void insertThreadToFila(int prio, void * thread) {
  switch (prio) {
    case 0:
      insertFILA2((PFILA2) &controlBlock.aptoThreads, thread);
      break;
    case 1:
      insertFILA2((PFILA2) &controlBlock.blockedThreads, thread);
      break;
    default:
      break;
  }
}

void removeThreadFromFila(int prio, int tid) {
  switch (prio) {
    case 0:
      removeFILA2((PFILA2) &controlBlock.aptoThreads, tid);
      break;
    case 1:
      removeFILA2((PFILA2) &controlBlock.blockedThreads, tid);
      break;
    default:
      break;
  }
}

int generateTID(void) {
	static int globalTID = 0;

	return ++globalTID;
}

int scheduler(void) {
  TCB_t* nextRunningThread;

  if (FirstFila2((PFILA2) &controlBlock.aptoThreads) == 0) {
    nextRunningThread = (TCB_t*) GetAtIteratorFila2((PFILA2) &controlBlock.aptoThreads);
    removeThreadFromFila(0, nextRunningThread->tid);
  } else if (FirstFila2((PFILA2) &controlBlock.blockedThreads) == 0) {
    nextRunningThread = (TCB_t*) GetAtIteratorFila2((PFILA2) &controlBlock.blockedThreads);
    removeThreadFromFila(1, nextRunningThread->tid);
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

  	insertThreadToFila(currentThread->prio, (void *) currentThread);
  	controlBlock.runningThread = nextRunningThread;

  	swapcontext(&currentThread->context, &nextRunningThread->context);
  	return 0;
}
