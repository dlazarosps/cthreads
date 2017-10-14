#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdata.h"
#include "cfila.h"
#include "cthread.h"
#include "support.h"

struct control controlBlock = { .initiated = FALSE };


/**
	Função para criação de uma thread. Exige a alocação de estruturas
	(TCB) para sua gerência correta. Ao ser criada, ela é inserida 
	na fila de aptos.
	
	@param void* (*start) (void*) - ponteiro para a função que a 
	thread executará
	@param void* arg Um parâmetro que pode ser passado para 
	a thread na criação
	@return Retorna um valor positivo caso executado CORRETAMENTE,
	retornando o identificador da thread criada. E negativo caso 
	contrário.
*/
int ccreate (void* (*start)(void*), void *arg, int prio) {
  if (!controlBlock.initiated) {
    cinit();
  }

  TCB_t* newThread;
  newThread = (TCB_t*) malloc(sizeof(TCB_t));
  newThread->tid = generateTID();
  newThread->state = PROCST_APTO;
  newThread->prio = prio;

  getcontext(&newThread->context);

  newThread->context.uc_link = &controlBlock.endThread;
  newThread->context.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
  newThread->context.uc_stack.ss_size = SIGSTKSZ;

  if(newThread->context.uc_stack.ss_sp == NULL) {
    return -1;
  }

  makecontext(&newThread->context, (void(*))start, 1, arg);

  insertFILA2(&controlBlock.allThreads, (void *) newThread);
  insertThreadToFila(prio, (void *) newThread);

  return newThread->tid;
};

/*
int csetprio(int tid, int prio) {
  if (!controlBlock.initiated) {
    cinit();
  }
  PFILA2 allThreads = &controlBlock.allThreads;

  if(searchFILA2(allThreads, tid, TRUE) == TRUE) {
    TCB_t* copyThread = (TCB_t*) GetAtIteratorFila2(allThreads);

    int oldprio = copyThread->prio;
    copyThread->prio = prio;

    removeThreadFromFila(oldprio, tid);
    insertThreadToFila(prio, (void *) copyThread);

    return 0;
  } else {
    return -1;
  }
};
*/

/**
	Função que retorna uma thread ao estado de apto. Uma thread pode 
	liberar a CPU de forma voluntária com o auxilio da primitiva Cyield.
	Com isto, o escalonador será chamado para selecionar a thread que 
	receberá a CPU.
	
	@return Retorna ZERO quando executado CORRETAMENTE. E valor negativo
	caso contrário.
	
*/
int cyield(void) {
  if (!controlBlock.initiated) {
    cinit();
  }

  TCB_t* lastRunningThread = controlBlock.runningThread;
  lastRunningThread->state = PROCST_APTO;

  insertThreadToFila(lastRunningThread->prio, lastRunningThread);
  scheduler();

  return 0;
};


/**
	Uma thread pode ser bloqueada até que outra termine sua execução usando a função cjoin.
	A função cjoin recebe como parâmetro o identificador da thread cujo término está sendo aguardado. Quando essa
	thread terminar, a função cjoin retorna com um valor inteiro indicando o sucesso ou não de sua execução. Uma
	determinada thread só pode ser esperada por uma única outra thread. Se duas ou mais threads fizerem cjoin para uma
	mesma thread, apenas a primeira que realizou a chamada será bloqueada. As outras chamadas retornarão
	imediatamente com um código de erro e seguirão sua execução. Se cjoin for feito para uma thread que não existe (não
	foi criada ou já terminou), a função retornará imediatamente com um código de erro. Observe que não há necessidade
	de um estado zombie, pois a thread que aguarda o término de outra (a que fez cjoin) não recupera nenhuma
	informação de retorno proveniente da thread aguardada.
	
	@param tid Identificador da Thread que está aguardando seu término
	@return Retorna ZERO quando executado corretamente. E valor negativo caso 
	contrário.

*/
int cjoin(int tid) {
  if (!controlBlock.initiated) {
    cinit();
  }

  /*TO DO*/
  //Código e lógica de implementação correta
  
  /* troca de contexto */
  scheduler();

  return 0;
};


/**
	Função csem_init inicializa uma variável do tipo csem_t e consiste 
	em fornecer um valor inteiro (count), positivo ou negativo, que 
	representa a quantidade existente do recurso controlado pelo semáforo.
	Para realizar exclusão mútua, esse valor inicial da variável semáforo
	deve ser 1 (semáforo binário). Ainda, cada variável	semáforo deve 
	ter associado uma estrutura que registre as threads que estão bloqueadas, 
	esperando por sua liberação. Na inicialização essa lista deve estar vazia.
	
	@param csem_t *sem Ponteiro para uma variável do tipo csem_t. Aponta para 
	uma estrutura de dados que representa a variável semaforo.
	@param count Valor a ser usado na inicialização do semáforo. Representa a 
	quantidade de recursos controlado pelo semaforo.
	@return Retorna ZERO quando executado CORRETAMENTE. E valor negativo caso
	contrario.
*/
int csem_init(csem_t *sem, int count) {
  if (!controlBlock.initiated) {
    cinit();
  }

  sem->count = count;
  sem->fila = (PFILA2) malloc(sizeof(PFILA2));

  if (CreateFila2(sem->fila) == 0) {
    return 0;
  } else {
    return -1;
  }
};


/**
	Função usada para solicitar um recurso. Se o recurso estiver livre, ele é
	atribuído a thread, que continuará a sua execução normalmente; caso 
	contrário a thread será bloqueada e posta a espera desse recurso na fila. 
	Se na chamada da função o valor de count for menor ou igual a zero, a thread 
	deverá ser posta no estado bloqueado e colocada na fila associada a variável 
	semáforo. Para cada chamada a cwait a variável count da estrutura semáforo é 
	decrementada de uma unidade.
	
	@param csem_t *sem Ponteiro para uma variavel do tipo semaforo.
	@return Retorna ZERO quando executado CORRETAMENTE. E valor negativo caso 
	contrario.
*/
int cwait(csem_t *sem) {
  if (!controlBlock.initiated) {
    cinit();
  }

  TCB_t* RunningThread = controlBlock.runningThread;

  if(sem->count > 0){
    sem->count = 0;
    return 0;
  }
  else{
        if(AppendFila2(sem->fila, (void *) RunningThread)==0){
            sem->count--;
            RunningThread->state = PROCST_BLOQ;
            scheduler();
            return 0;
        }
  }
    return -1;
};


/**
	Função que serve para indicar que a thread está liberando o recurso. Para cada 
	chamada da primitiva csignal, a variável count deverá ser incrementada de uma 
	unidade. Se houver mais de uma thread bloqueada a espera desse recurso a primeira
	delas, segundo uma política de FIFO, deverá passar para o estado apto e as demais
	devem continuar no estado bloqueado.
	
	@param csem_t *sem Ponteiro para uma variável do tipo semaforo.
	@return Retorna ZERO quando executado CORRETAMENTE. E valor negativo caso 
	contrario.
*/
int csignal(csem_t *sem) {
  if (!controlBlock.initiated) {
    cinit();
  }
  sem->count++;
  if (sem->count>0){
    return 0;
  }
  else{
    FirstFila2(sem->fila);
    TCB_t *aux;
    aux = GetAtIteratorFila2(sem->fila);
    if (aux==NULL){
        return -1;
    }
    DeleteAtIteratorFila2(sem->fila);
    aux->state = PROCST_APTO;
    insertThreadToFila(aux->prio, (void *) aux);
    return 0;
  }
};


/**
	Função para identificação de cada um dos integrantes do grupo.
	
	@param *name  Ponteiro para uma área de memória onde deve ser escrito 
	um string que contém os nomes dos componentes do grupo e seus números
	de cartão. Deve ser uma linha por componente.
	@param size Quantidade máxima de caracteres que podem ser copiados para o 
	string de identificação dos componentes do grupo.
	@return Retorna ZERO quando executado CORRETAMENTE. E valor negativo caso 
	contrario. 
*/
int cidentify (char *name, int size) {
  if (!controlBlock.initiated) {
    cinit();
  }

  if (size == 0) {
    name[0] = '\0';

    return 0;
  }

  char info[] = "\n Douglas Lazaro S P Silva \t- 207305 \n Henrique La Porta \t- 273112 \n Rodrigo Okido \t- 252745";
  int length = strlen(info);

  if(size < length) {
    memcpy(name, info, size);
    name[size] = '\0';
  } else {
    memcpy(name, info, length);
    name[length] = '\0';
  }

  return 0;
};
