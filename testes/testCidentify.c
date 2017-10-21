#include <stdio.h>
#include <stdlib.h>
#include "../include/cthread.h"
#include "../include/cfila.h"
#include "../include/cdata.h"
#include "../include/support.h"


void* func0(void *arg) {
	printf("Eu sou a thread ID0 imprimindo %d\n", *((int *)arg));
	return;
}

void* func1(void *arg) {
	printf("Eu sou a thread ID1 imprimindo %d\n", *((int *)arg));
}


int main() {

  printf("\t \t **** INICIO DA SESSÃO DE TESTES **** ");
  
  printf("\n >> Teste da CIDENTIFY << \n");
  printf("\t \t CINIT = %d \n", cinit());
  
  int size = 93;
  char *name = malloc(sizeof(size));
  
  cidentify(name,size);
  
  // ** FIM DO TESTE DA CIDENTIFY **
  
  // >> INICIANDO A CRIAÇÃO DE DUAS THREAD PELA CCREATE << (CIDENTIFY + CCREATE)
  // Código aproveitado do exemplo.c localizado no diretório "exemplos" do projeto.
	int	id0, id1;
	int i;
	
	printf("\n >> Geração de duas threads através da CCREATE << \n");
	id0 = ccreate(func0, (void *)&i, 0);
	printf("THREAD 1 criada: - TID %d \n", id0);
	id1 = ccreate(func1, (void *)&i, 1);
	printf("THREAD 2 criada: - TID %d \n", id1);
	
	
  // ** FIM DO TESTE CIDENTIFY + CCREATE **
  
  // >> LIBERANDO A CPU DE UMA THREAD USANDO A CYIELD
	printf("\n >> Liberação da CPU de uma thread com a CYIELD << \n");
	printf("Selecionando uma thread na fila de aptos: %d \n", scheduler());
	printf("Liberação da CPU (0 = SUCESSO): %d", cyield());
	
	
  

  
  

  return 0;
}