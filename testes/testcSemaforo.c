#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/cthread.h"
#include "../include/cfila.h"
#include "../include/cdata.h"
#include "../include/support.h"


void* func0(void *arg) {
	printf("Eu sou a thread ID0 imprimindo %d\n", *((int *)arg));
	// return;
}

void* func1(void *arg) {
	printf("Eu sou a thread ID1 imprimindo %d\n", *((int *)arg));
	// return;
}


int main() {

	printf("\t \t **** INICIO DA SESSÃO DE TESTES **** ");
  
	printf("\n >> Teste de SEMÁFOROS << \n");
	printf("\t \t CINIT = %d \n", cinit());
 
	// >> INICIANDO A CRIAÇÃO DE DUAS THREAD PELA CCREATE << (CCREATE)
	// Código aproveitado do exemplo.c localizado no diretório "exemplos" do projeto.
	int	id0, id1;
	int i;
	
	printf("\n >> Geração de duas threads através da CCREATE << \n");
	id0 = ccreate(func0, (void *)&i, 0);
	printf("THREAD 1 criada: - TID %d \n", id0);

	id1 = ccreate(func1, (void *)&i, 0);
	printf("THREAD 2 criada: - TID %d \n", id1);
	
	// >> INICIANDO A CRIAÇÃO DE UM SEMAFORO << (CCREATE + CSEM_INIT)
	printf("\n >> Geração de um Semaforo << \n");
	csem_t *semaforo;
	semaforo = (csem_t*) malloc(sizeof(csem_t));
	printf("Inicializando semaforo (SUCESSO = 0) : %d \n", csem_init(semaforo, -1));
	
	TCB_t* thread;
	if(FirstFila2((PFILA2) &controlBlock.aptoThreads) == 0){
		thread = (TCB_t*) GetAtIteratorFila2((PFILA2) &controlBlock.aptoThreads);
		printf("SELECIONANDO A THREAD: %p\n", thread->tid);
		removeFILA2((PFILA2) &controlBlock.aptoThreads, id0);
		if (insertFILA2((PFILA2) &semaforo->fila, thread) == 0){
				// >> CHAMADA DA CSIGNAL PARA LIBERACAO DE RECURSO << (CCREATE + CSEM_INIT + CSIGNAL)
			printf("Thread liberando recurso: %d \n", csignal(semaforo)); 	//Insere a thread 1 em bloqueados
		}
	}
	

	printf("\n **** FIM DO TESTE DE SEMAFORO ****\n");

  return 0;
}