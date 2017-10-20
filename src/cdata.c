#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "cfila.h"
#include "cdata.h"
#include "cthread.h"
#include "support.h"


static int globalTID = 0;

/**
  Função de inicialização da control.
*/
int cinit(void) {
  int check;

  /*
    Initiate control block elements
  */
  controlBlock.initiated = TRUE;
  controlBlock.isfirst = TRUE;

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

  // Verifica threads bloqueadas que estavam esperando thread que terminou

  /* RASCUNHO */

  TCB_t* unblocked; 

  if (FirstFila2((PFILA2) &controlBlock.blockedThreads)==0) {
    do{
        
      unblocked = (TCB_t *) GetAtIteratorFila2((PFILA2) &controlBlock.blockedThreads);  
        
      //Caso o tidjoin for igual ao tid da thread que terminou desbloqueia
      if (unblocked->tidJoinWait == controlBlock.runningThread->tid) {

        unblocked->prio = PROCST_APTO;
        insertByPrio((PFILA2) &controlBlock.aptoThreads, unblocked);

      }
    }
    while (NextFila2((PFILA2) &controlBlock.blockedThreads)==0);
  }
  /* -- */

    //Põe a rodar a proxima thread
  	scheduler();
}

/**
	Função para gerar um TID a uma thread. Cada thread deverá ser associada a
	um identificador único (TID – thread identifier) que será um número inteiro
	positivo (com sinal), de 32 bits (int).

*/
int generateTID(void) {
	return ++globalTID;
}


/**
	Função que seleciona na fila de threads a proxima thread a ser executada.

*/
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


/**
	Função que executa a troca de contexto. Salva o tempo de inicio da thread que entra
	em execução, e altera o valor da prioridade da thread ao sair da execução.

*/
int dispatcher(TCB_t* nextRunningThread){
  TCB_t* currentThread = controlBlock.runningThread;

  /*Caso dispatcher esteja rodando pela primeira vez */
  if (controlBlock.isfirst == TRUE){

    currentThread->state = PROCST_APTO;
    currentThread->prio = currentThread->prio+1;
    insertByPrio((PFILA2) &controlBlock.aptoThreads, currentThread);
    
    //Desativa flag
    controlBlock.isfirst = FALSE;

  }
  else{
    /* controle para:
    - não alterar o valor da prioridade incorretamente
    - e inserir na fila correta
    */

    //Altera a prioridade da thread
    currentThread->prio = currentThread->prio + stopTimer();
    
    //Caso a thread esteja é apta ou estava executando insere na fila de aptas
    if(currentThread->state == PROCST_APTO || currentThread->state == PROCST_EXEC){

      currentThread->state = PROCST_APTO;
      insertByPrio((PFILA2) &controlBlock.aptoThreads, currentThread);

    }

    // Caso a thread esteja foi bloqueda insere na fila de bloqueadas
    if(currentThread->state == PROCST_BLOQ){

      insertFILA2(controlBlock.blockedThreads, (void *) currentThread);

    }  
  }

  //efetua a troca de contexto running <-> next
  controlBlock.runningThread = nextRunningThread;
  swapcontext(&currentThread->context, &nextRunningThread->context);

  //inicia o timer da prioridade
  startTimer();

	return 0;
}

//Copiada da /src/insert.c
/*--------------------------------------------------------------------
Função: Insere um nodo na lista indicada, segundo o campo "prio" do TCB_t
  A fila deve estar ordenada (ou ter sido construída usado apenas essa funcao)
  O primeiro elemento da lista (first) é aquele com menor vamor de "prio"
Entra:  pfila -> objeto FILA2
  pnodo -> objeto a ser colocado na FILA2
Ret:  ==0, se conseguiu
  !=0, caso contrário (erro)
--------------------------------------------------------------------*/
int insertByPrio(PFILA2 pfila, TCB_t *tcb) {
  TCB_t *tcb_it;
  
  // pfile vazia?
  if (FirstFila2(pfila)==0) {
    do {
      tcb_it = (TCB_t *) GetAtIteratorFila2(pfila);
      if (tcb->prio < tcb_it->prio) {
        return InsertBeforeIteratorFila2(pfila, tcb);
      }
    } while (NextFila2(pfila)==0);
  } 
  return AppendFila2(pfila, (void *)tcb);
}