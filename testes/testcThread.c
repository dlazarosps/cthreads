#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/cthread.h"
#include "../include/cfila.h"
#include "../include/cdata.h"
#include "../include/support.h"


void* func0(void *arg) {
	printf("[RODANDO] - função - thread TID:[%d] - 1 vez \n", controlBlock.runningThread->tid);
	return NULL;
}

void* func1(void *arg) {
	printf("[RODANDO] - função - thread TID:[%d] - 1 vez \n", controlBlock.runningThread->tid);
	cyield();
	printf("[RODANDO] - função - thread TID:[%d] - 2 vez \n", controlBlock.runningThread->tid);
	cyield();
	return NULL;
}

void* func2(void *arg) {
	printf("[RODANDO] - função - thread TID:[%d] - 1 vez \n", controlBlock.runningThread->tid);
	cyield();
	printf("[RODANDO] - função - thread TID:[%d] - 2 vez \n", controlBlock.runningThread->tid);
	cyield();
	return NULL;
}

int main() {

  printf("\t \t **** INICIO DA SESSÃO DE TESTES **** ");

  printf("\n >> Teste da CIDENTIFY << \n");
  printf("\t \t CINIT = %d \n", cinit());

  printf("\t \t runningThread = %d \n", controlBlock.runningThread->tid);

  int size = 93;
  char *name = malloc(sizeof(size));

  cidentify(name,size);

  // ** FIM DO TESTE DA CIDENTIFY **

  // >> INICIANDO A CRIAÇÃO DE DUAS THREAD PELA CCREATE << (CIDENTIFY + CCREATE)
  // Código aproveitado do exemplo.c localizado no diretório "exemplos" do projeto.
	int	id0, id1, id2;
	int i;

	printf("\n >> Geração de duas threads através da CCREATE << \n");
	id0 = ccreate(func0, (void *)&i, 0);
	printf("THREAD 1 criada: - TID %d \n", id0);

	// scheduler();

	id1 = ccreate(func1, (void *)&i, 0);
	printf("THREAD 2 criada: - TID %d \n", id1);


	id2 = ccreate(func2, (void *)&i, 0);
	printf("THREAD 3 criada: - TID %d \n", id2);

	printf("\n ----------- \n");
	sleep(1);


	TCB_t * node;
	if(FirstFila2((PFILA2) &controlBlock.aptoThreads) == 0){
		node = (TCB_t*) GetAtIteratorFila2((PFILA2) &controlBlock.aptoThreads);
		printf("\t FIRST APTAS \n");
		printf("Pointer: %p - TID: %d - Prio = %d \n", node, node->tid, node->prio);
	}
	else{
		printf("\n -----ERRO FIRST APTOS------ \n");
	}

	if(FirstFila2((PFILA2) &controlBlock.allThreads) == 0){
		node = (TCB_t*) GetAtIteratorFila2((PFILA2) &controlBlock.aptoThreads);
		printf("\t FIRST ALL \n");
		printf("Pointer: %p - TID: %d - Prio = %d \n", node, node->tid, node->prio);
	}
	else{
		printf("\n -----ERRO FIRST ALL------ \n");
	}

	printf("\n ----------- \n");

	// sleep(1);


  // ** FIM DO TESTE CIDENTIFY + CCREATE **

  // >> ATRIBUINDO CJOIN AS THREADS RECEM GERADAS << (CIDENTIFY + CCREATE + CJOIN)
	// printf("CJOIN na THREAD 1 (SUCESSO = 0): %d \n", cyield());
	printf("CJOIN na THREAD 0 (SUCESSO = 0): %d \n", cjoin(id1));


  // ** FIM DO TESTE CIDENTIFY + CCREATE + CJOIN **


  // >> LIBERANDO A CPU DE UMA THREAD USANDO A CYIELD << (CIDENTIFY + CCREATE + SCHEDULER + CYIELD)
	printf("\n >> Liberação da CPU de uma thread com a CYIELD << \n");
  //printf("Selecionando uma thread na fila de aptos: %d \n", scheduler());
	printf("Liberação da CPU (0 = SUCESSO): %d \n", cyield());

  // ** FIM DO TESTE CIDENTIFY + CCREATE + CJOIN +CYIELD **







  return 0;
}
