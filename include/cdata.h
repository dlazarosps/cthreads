/*
 * cdata.h: arquivo de inclusão de uso apenas na geração da libpithread
 *
 * Esse arquivo pode ser modificado. ENTRETANTO, deve ser utilizada a TCB fornecida
 *
 * Versão de 11/09/2017
 *
 */
#ifndef __cdata__
#define __cdata__
#include "ucontext.h"

#define	PROCST_CRIACAO	0
#define	PROCST_APTO	1
#define	PROCST_EXEC	2
#define	PROCST_BLOQ	3
#define	PROCST_TERMINO	4

/* Os campos "tid", "state", "prio" e "context" dessa estrutura devem ser mantidos e usados convenientemente
   Pode-se acrescentar outros campos APÓS os campos obrigatórios dessa estrutura
*/
typedef struct s_TCB { 
	int		tid; 		// identificador da thread
	int		state;		// estado em que a thread se encontra
					// 0: Criação; 1: Apto; 2: Execução; 3: Bloqueado e 4: Término
	unsigned 	int		prio;		// prioridade da thread (higest=0; lowest=3)
	ucontext_t 	context;	// contexto de execução da thread (SP, PC, GPRs e recursos) 
	
	/* Se necessário, pode-se acresecentar campos nessa estrutura A PARTIR DAQUI! */
	
	
} TCB_t; 


/* Definido pelo grupo */
#define FALSE 0
#define TRUE 1

#define DEBUG FALSE

#define prio ticket

struct control {
	int initiated;

	FILA2 allThreads;
	PFILA2 aptoThreads
	PFILA2 blockedThreads
	
	TCB_t* runningThread;
	ucontext_t endThread;
};

extern struct control controlBlock;

typedef struct Pjoin{
	TCB_t* waiting;
	TCB_t* awaited;
} Pjoin;


int cinit(void);
void endThread(void);
void insertThreadToFila(int prio, void * thread);
void removeThreadFromFila(int prio, int tid);
int generateTID(void);

int scheduler(void);
int dispatcher(TCB_t *nextRunningThread);

#endif