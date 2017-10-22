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

  if (insertFILA2(&controlBlock.allThreads, (void *) mainThread) != 0){
    #if DEBUG
      printf("[ERRO] cinit - insert - Não inserida em allThreads \n");
    #endif
    return -4;
  }

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
  controlBlock.runningThread = mainThread;

  getcontext(&mainThread->context);
  mainThread->context.uc_link =  &controlBlock.endThread;
  mainThread->context.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
  mainThread->context.uc_stack.ss_size = SIGSTKSZ;

  return 0;
};


void endThread(void){
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
	if (controlBlock.runningThread->tidJoinWait >= 0){
	TCB_t* unblocked; 
		//procura thread na fila allThreads
		  if(searchFILA2(&controlBlock.allThreads, controlBlock.runningThread->tidJoinWait, TRUE) == TRUE) {
			unblocked = (TCB_t*) GetAtIteratorFila2(&controlBlock.allThreads);
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
	#if DEBUG
  	printf("[scheduler] - init \n");
	#endif
  TCB_t* nextRunningThread;
  if (FirstFila2((PFILA2) &controlBlock.aptoThreads) == 0) {
    nextRunningThread = (TCB_t*) GetAtIteratorFila2((PFILA2) &controlBlock.aptoThreads);
    if(removeFILA2((PFILA2) &controlBlock.aptoThreads, nextRunningThread->tid) != TRUE){
    
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

    //caso não tenha nenhuma thread apta continua executando a q estava
    nextRunningThread = controlBlock.runningThread;

    // return -2;
  }
  nextRunningThread->state = PROCST_EXEC;
	#if DEBUG
  	printf("[scheduler] - call dispatcher \n");
	#endif
  return   dispatcher(nextRunningThread);
}


/**
	Função que executa a troca de contexto. Salva o tempo de inicio da thread que entra
	em execução, e altera o valor da prioridade da thread ao sair da execução.

*/
int dispatcher(TCB_t* nextRunningThread){
	#if DEBUG
  	printf("[DISPATCHER] - init \n");
	#endif
  TCB_t* currentThread = (TCB_t*) &controlBlock.runningThread;

  /*Caso dispatcher esteja rodando pela primeira vez */
  if (controlBlock.isfirst == TRUE){
    currentThread->state = PROCST_APTO;
    // currentThread->prio = currentThread->prio+1;
    
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
        if(insertByPrio((PFILA2) &controlBlock.aptoThreads, currentThread)!=0){
          #if DEBUG
            printf("[ERRO] dispatcher - CASE default - Não inserida em Aptos \n");
          #endif
          return -3;
        }
    }  
  }

  controlBlock.runningThread = nextRunningThread;
  //inicia o timer da prioridade
  startTimer();
  
  //efetua a troca de contexto running <-> next
  swapcontext(&currentThread->context, &nextRunningThread->context);

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
