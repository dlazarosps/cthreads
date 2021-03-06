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
  // TCB_t *mainThread;

  controlBlock.mainThread = (TCB_t*) malloc(sizeof(TCB_t));
  controlBlock.mainThread->tid = 0;
  controlBlock.mainThread->state = PROCST_EXEC;
  controlBlock.mainThread->prio = 0;

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
  getcontext(&controlBlock.mainThread->context);
  controlBlock.mainThread->context.uc_link =  &controlBlock.endThread;
  controlBlock.mainThread->context.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
  controlBlock.mainThread->context.uc_stack.ss_size = SIGSTKSZ;
  // makecontext(&controlBlock.mainThread->context, (void (*)(void))endThread, 0);

 //insere na allthreads só após copia de contexto
  controlBlock.runningThread = controlBlock.mainThread;

  if (insertFILA2(&controlBlock.allThreads, (void *) controlBlock.runningThread) != 0){
    #if DEBUG
      printf("[ERRO] cinit - insert - Não inserida em allThreads \n");
    #endif
    return -4;
  }

  return 0;
};


void endThread(void){
	#if DEBUG
      printf("[ENDTHREAD] Thread - TID[%d] - STATE[%d] \n",controlBlock.runningThread->tid, controlBlock.runningThread->state);
    #endif
	getcontext(&controlBlock.endThread);

	// Caso exista desbloquei a thread que esperava o seu término
	releaseThreadJoin();

	// Delete thread from blocking threads
	controlBlock.runningThread->state = PROCST_TERMINO;

	#if DEBUG
  	printf("TID: %i has ended. \n", controlBlock.runningThread->tid);
	#endif

    //Põe a rodar a proxima thread
	scheduler();
}

void releaseThreadJoin(void){
	#if DEBUG
      printf("[releaseThreadJoin] unblocking - TID[%d]\n",controlBlock.runningThread->tidJoinWait);
    #endif
	if (controlBlock.runningThread->tidJoinWait >= 0){
	TCB_t* unblocked;
		//procura thread na fila blockedThreads
		  if(searchFILA2((PFILA2) &controlBlock.blockedThreads, controlBlock.runningThread->tidJoinWait, TRUE) == TRUE) {
			unblocked = (TCB_t*) GetAtIteratorFila2((PFILA2) &controlBlock.blockedThreads);
			if(unblocked->prio == PROCST_BLOQ){
				if(DeleteAtIteratorFila2((PFILA2) &controlBlock.blockedThreads)== 0){ //Remove da fila de Blocked
					unblocked->prio = PROCST_APTO;
					insertByPrio((PFILA2) &controlBlock.aptoThreads, unblocked); //Adiciona na fila de APTO
				}
			}else{
				#if DEBUG
				printf("[ERRO] releaseThreadJoin - a thread não esta no estao bloqueado \n");
				#endif
			}
		  } else {
			#if DEBUG
			printf("[ERRO] releaseThreadJoin - a thread não encontrada na allThreads \n");
			#endif
		  }
   }
  #if DEBUG
  	printf("[releaseThreadJoin] - finish \n");
	#endif
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
  setcontext(&controlBlock.mainThread->context);
	#if DEBUG
    printf("[SCHEDULER]- RunningThread - TID[%d] - PRIO[%d] \n", controlBlock.runningThread->tid, controlBlock.runningThread->prio);
	#endif
  TCB_t* nextRunningThread;
  if (FirstFila2((PFILA2) &controlBlock.aptoThreads) == 0) {
    nextRunningThread = (TCB_t*) GetAtIteratorFila2((PFILA2) &controlBlock.aptoThreads);
    if(DeleteAtIteratorFila2((PFILA2) &controlBlock.aptoThreads) != 0){

      #if DEBUG
        printf("[ERRO] scheduler - removeFILA2 - Não removida \n");
      #endif

      return -1;
    }

  }
  else {

  	#if DEBUG
  	printf("[ERRO] scheduler - Não encontrada nenhuma thread Apta\n");
  	#endif

     if (controlBlock.runningThread->state != PROCST_TERMINO)		 
     {		
       nextRunningThread = controlBlock.runningThread;		
     }		
     else{		
       return -2;		
     }


  }
  nextRunningThread->state = PROCST_EXEC;
	#if DEBUG
  	printf("[scheduler] - call dispatcher (nextRunningThread->tid: %d) \n",nextRunningThread->tid);
	#endif
  return   dispatcher(nextRunningThread);
}


/**
	Função que executa a troca de contexto. Salva o tempo de inicio da thread que entra
	em execução, e altera o valor da prioridade da thread ao sair da execução.

*/
int dispatcher(TCB_t* nextRunningThread){
  TCB_t* currentThread = (TCB_t*) &controlBlock.runningThread;
	#if DEBUG
    printf("[DISPATCHER] CurrentThread - TID[%d] - PRIO[%d] <-> NextRunningThread - TID[%d] - PRIO[%d]\n", currentThread->tid, currentThread->prio, nextRunningThread->tid, nextRunningThread->prio);
    #endif
  /*Caso dispatcher esteja rodando pela primeira vez */
  if (controlBlock.isfirst == TRUE){
    currentThread->state = PROCST_APTO;
    currentThread->prio = currentThread->prio+1;

    if(insertByPrio((PFILA2) &controlBlock.aptoThreads, currentThread) != 0){
      #if DEBUG
        printf("[ERRO] dispatcher - FIRST - Não inserida em Aptos \n");
      #endif
      return -1;
    }
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
    startTimer();
    controlBlock.runningThread = nextRunningThread;
    //inicia o timer da prioridade

    #if DEBUG
    printf("[DISPATCHER]- CurrentThread New Prio- TID[%d] - PRIO[%d] \n", currentThread->tid, currentThread->prio);
    #endif

    switch(currentThread->state){

      case PROCST_TERMINO:
        #if DEBUG
        printf("[DISPATCHER] - CASE PROCST_TERMINO \n");
        #endif
        //deletar thread da allthreads ?
        //libera pilha do contexto
        free(currentThread->context.uc_stack.ss_sp);
        free(currentThread);

        if (nextRunningThread != currentThread){ //se for diferente

          startTimer();
          setcontext(&nextRunningThread->context); //contexto da proxima
          return 0;
        }
        break;
    // Caso a thread esteja foi bloqueda insere na fila de bloqueadas
      case PROCST_BLOQ:
        #if DEBUG
        printf("[DISPATCHER] - CASE PROCST_BLOQ \n");
        #endif
        if(insertFILA2((PFILA2) &controlBlock.blockedThreads, (void *) currentThread) != 0){
          #if DEBUG
            printf("[ERRO] dispatcher - CASE BLOQ - Não inserida em blockedThreads \n");
          #endif
            return -2;
        }
        break;

    //Caso a thread esteja é apta ou estava executando insere na fila de aptas
      case PROCST_APTO:
      case PROCST_EXEC:
      default:
        #if DEBUG
        printf("[DISPATCHER] - CASE PROCST_APTO PROCST_EXEC DEFAULT\n");
        #endif
        currentThread->state = PROCST_APTO;
        if(insertByPrio((PFILA2) &controlBlock.aptoThreads, (void *) currentThread)!=0){
          #if DEBUG
            printf("[ERRO] dispatcher - CASE default - Não inserida em Aptos \n");
          #endif
          return -3;
        }
        break;
    }
  }

  //efetua a troca de contexto running <-> next
  return swapcontext(&currentThread->context, &controlBlock.runningThread->context);
 
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
