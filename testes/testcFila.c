#include <stdio.h>
#include <stdlib.h>
#include "../include/cfila.h"
#include "../include/cdata.h"

void searchArrayTID(PFILA2 allThreads, int TIDs[], int size) {
	size_t j;

	for (j = 0; j < size; j++) {
		if(searchFILA2(allThreads, TIDs[j], TRUE) == TRUE) {
			TCB_t* copyThread = (TCB_t*) GetAtIteratorFila2(allThreads);
			printf("Pointer: %p - TID: %d\n", copyThread, copyThread->tid);
		} else {
			printf("TID %d not found.\n", TIDs[j]);
		}
	}
}

int main() {
  struct control controlBlock = { .initiated = FALSE };
  TCB_t *thread0, *thread1, *thread2, *thread3;

	int searchTID[6] = { 0, 1, 2, 3, 4, 5 };
	
	printf("\t \t CINIT = %d \n", cinit());

	PFILA2 allThreads = &controlBlock.allThreads;

	// Inicializando threads
  thread0 = (TCB_t*) malloc(sizeof(TCB_t));
  thread0->tid = 0;
  thread0->prio = 1;
  thread1 = (TCB_t*) malloc(sizeof(TCB_t));
  thread1->tid = searchTID[1];
  thread1->prio = 3;
  thread2 = (TCB_t*) malloc(sizeof(TCB_t));
  thread2->tid = searchTID[2];
  thread2->prio = 2;
  thread3 = (TCB_t*) malloc(sizeof(TCB_t));
  thread3->tid = searchTID[3];
  thread0->prio = 4;

  // Inserção na fila
  insertFILA2(allThreads, (void *) thread0);
  insertFILA2(allThreads, (void *) thread1);
  insertFILA2(allThreads, (void *) thread2);
  insertFILA2(allThreads, (void *) thread3);

  // Pesquisa usando o TID, dentro da fila.
	printf("Efetuando buscas por TID.\n");
	searchArrayTID(allThreads, searchTID, 6);

	/*
	// Remoção de elemento da fila
	removeFILA2(allThreads, searchTID[5]);
	removeFILA2(allThreads, searchTID[2]);

	// Pesquisa usando os mesmos TIDs anteriores.
	printf("\nElementos com TID %d e TID %d removidos. Efetuando busca novamente.\n", searchTID[5], searchTID[2]);
	searchArrayTID(allThreads, searchTID, 6);

	// Inserção novamento dos elementos removidos.
	insertFILA2(allThreads, (void *) thread1);
	insertFILA2(allThreads, (void *) thread3);

	// Pesquisa usando os mesmos TIDs anteriores.
	printf("\nElementos TID %d e TID %d reinseridos. Efetuando busca...\n", searchTID[5], searchTID[2]);
	searchArrayTID(allThreads, searchTID, 6);
	*/

	printf("\n --------------------------- \n");
	printf("\n ---Testando Fila de Aptas--- \n");
	printf("\n --------------------------- \n");
	
	printf("\n -----0|1--1|3--2|2--3|4----- \n");

	if(insertThreadToFila(thread0->prio, (void *) thread0) == 1){
		printf("\t Primeira Thread inserida no inicio \n");
	}
/*	
	if(insertThreadToFila(thread1->prio, (void *) thread1) == 2){
		printf("\t Segunda Thread inserida (apos o laço) \n");
	}*/
	// insertThreadToFila(thread1->prio, thread1);
	// insertThreadToFila(thread2->prio, thread2);
	// insertThreadToFila(thread3->prio, thread3);

	TCB_t* testeThread;

	if (FirstFila2((PFILA2) &controlBlock.aptoThreads) == 0) {
    	
    	//Primeira thread
    	testeThread = (TCB_t*) GetAtIteratorFila2((PFILA2) &controlBlock.aptoThreads);
    	printf("TID %d - Prio %d \n", testeThread, testeThread->tid, testeThread->prio);

    	/*//segunda thread
    	testeThread = (TCB_t*) GetAtNextIteratorFila2((PFILA2) &controlBlock.aptoThreads);
    	printf("TID %d - Prio %d \n",testeThread->tid, testeThread->prio);

    	//ultima thread
    	if (LastFila2((PFILA2) &controlBlock.aptoThreads) == 0) {

			testeThread = (TCB_t*) GetAtIteratorFila2((PFILA2) &controlBlock.aptoThreads);
    		printf("TID %d - Prio %d \n",testeThread->tid, testeThread->prio);    	
		}*/
	}
	else{
		printf("\n ERRO ao pegar o First Apto \n");
	}

	printf("\n --------------------------- \n");
	printf("\n Efetuando buscas por TID APTOS.\n");
	searchArrayTID(&controlBlock.aptoThreads, searchTID, 6);




  return 0;
}
