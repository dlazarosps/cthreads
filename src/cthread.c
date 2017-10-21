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

  TCB_t* newThread; // Alocação para a criação de uma nova thread.
  newThread = (TCB_t*) malloc(sizeof(TCB_t));
  newThread->tid = generateTID(); // Gera um TID exclusivo a thread
  newThread->state = PROCST_APTO; // Coloca um estado a thread
  newThread->prio = prio; // Prioridade da thread.
  newThread->tidJoinWait = -1; 

  getcontext(&newThread->context);

  newThread->context.uc_link = &controlBlock.endThread;
  newThread->context.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
  newThread->context.uc_stack.ss_size = SIGSTKSZ;

  if(newThread->context.uc_stack.ss_sp == NULL) {
    return -1;
  }

  makecontext(&newThread->context, (void(*))start, 1, arg);

  //Insere a nova thread na fila de todas as threads
  insertFILA2(&controlBlock.allThreads, (void *) newThread);
  
  //Insere a nova thread na fila de aptos de acordo com sua prioridade
  insertByPrio((PFILA2) &controlBlock.aptoThreads, newThread);

  //Retorno do TID da thread recem gerada.
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
    insertByPrio((PFILA2) &controlBlock.aptoThreads, copyThread);

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

  //Liberação voluntária da runningThread 
  //Seleciona a próxima thread a ser executada.
  //yield -> scheduler -> dispatcher -> next
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
	
	@param tid Identificador da Thread que o término está sendo aguardado
	@return Retorna ZERO quando executado corretamente. E valor negativo caso 
	contrário.

*/
int cjoin(int tid) {
  if (!controlBlock.initiated) {
    cinit();
  }
  
  TCB_t* currentThread = (TCB_t*) &controlBlock.runningThread;
  int found = FALSE;
  
  
  //se a thread procurada esta sendo executada
  if (controlBlock.runningThread->tid == tid) {
	printf("[ERRO] cjoin - a thread procurada esta sendo executada\n");
    return -1;
  }
  
  TCB_t* auxThread;
  TCB_t* waitThread;
  // Verifica se a thread existe
  if (FirstFila2((PFILA2) &controlBlock.aptoThreads)==0) {
    do {
      auxThread = (TCB_t *) GetAtIteratorFila2((PFILA2) &controlBlock.aptoThreads);
	  
      if (tid == auxThread->tid) {
		waitThread = auxThread;
        found = TRUE;
      }
    } while (NextFila2((PFILA2) &controlBlock.aptoThreads)==0 && found == FALSE);
  }
  
  if (FirstFila2((PFILA2) &controlBlock.blockedThreads)==0) {
    do {
      auxThread = (TCB_t *) GetAtIteratorFila2((PFILA2) &controlBlock.blockedThreads);
      if (tid == auxThread->tid) {
		waitThread = auxThread;
        found = TRUE;
      }
    } while (NextFila2((PFILA2) &controlBlock.blockedThreads)==0 && found == FALSE);
  }
  
  if(found == FALSE) return -1;
  
  //Se já existe uma thread aguardando o seu término
  if( waitThread->tidJoinWait >= 0 ){
	 printf("[ERRO] cjoin - já existe uma thread aguardando o seu término\n");
	 return -1; 
  }
  
  // sinaliza que existe uma Thread esperando pelo termino dela
  
  waitThread->tidJoinWait = controlBlock.runningThread->tid;
  
  controlBlock.runningThread->state = PROCST_BLOQ;  
  
  // troca de  threads / contexto
  return scheduler();
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

  if (initFILA2(sem->fila, TRUE)) {
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

  // Recebe a thread corrente da control.
  TCB_t* RunningThread = controlBlock.runningThread;

  // Verifica o valor de count. Se menor, a thread é bloqueada e colocada na fila 
  // de semaforos.
  if(sem->count > 0){
    sem->count = 0;
    return 0;
  }
  else{
        if(AppendFila2(sem->fila, (void *) RunningThread)==0){
            sem->count--; //Variável count cai uma unidade.
            RunningThread->state = PROCST_BLOQ; // Altera seu estado para BLOQUEADO.
            scheduler(); // Seleciona a próxima thread a ser executada.
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
    DeleteAtIteratorFila2(sem->fila); // Tira a thread da fila de semaforo.
    aux->state = PROCST_APTO;	// Coloca a thread no estado de APTO.
    insertByPrio((PFILA2) &controlBlock.aptoThreads, aux); // Insere a thread na fila de APTOS de acordo com a prioridade.
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

    return -1;
  }

  char info[] = "\n Douglas Lazaro S P Silva \t- 207305 \n Henrique La Porta \t- 273112 \n Rodrigo Okido \t- 252745";
  int length = strlen(info);

  if(size < length) {
    memcpy(name, info, size);
    name[size] = '\0';
	printf("%s\n",name);
  } else {
    memcpy(name, info, length);
    name[length] = '\0';
	printf("%s\n",name);
  }

  return 0;
};
