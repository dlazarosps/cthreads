/*
 * cdata.h: arquivo de inclus�o de uso apenas na gera��o da libpithread
 *
 * Esse arquivo pode ser modificado. ENTRETANTO, deve ser utilizada a TCB fornecida
 *
 * Vers�o de 11/09/2017
 *
 */
#ifndef __cdata__
#define __cdata__

#include <ucontext.h>
#include "support.h"

#define	PROCST_CRIACAO	0
#define	PROCST_APTO	1
#define	PROCST_EXEC	2
#define	PROCST_BLOQ	3
#define	PROCST_TERMINO	4

/* Os campos "tid", "state", "prio" e "context" dessa estrutura devem ser mantidos e usados convenientemente
   Pode-se acrescentar outros campos AP�S os campos obrigat�rios dessa estrutura
*/
typedef struct s_TCB { 
	int		tid; 		// identificador da thread
	int		state;		// estado em que a thread se encontra
					// 0: Cria��o; 1: Apto; 2: Execu��o; 3: Bloqueado e 4: T�rmino
	unsigned 	int		prio;		// prioridade da thread (higest=0; lowest=3)
	ucontext_t 	context;	// contexto de execu��o da thread (SP, PC, GPRs e recursos) 
	
	/* Se necess�rio, pode-se acresecentar campos nessa estrutura A PARTIR DAQUI! */
	
	int		tidJoinWait;  //-1: nenhuma Thread esperando o seu termino | Qualquer outro valor positivo: possui Thread esperando seu t�rmino
	
} TCB_t; 


/* Definido pelo grupo */
#define FALSE 0
#define TRUE 1

#define DEBUG FALSE

struct control {
	int initiated;
	int isfirst;

	FILA2 allThreads;
	
	PFILA2 aptoThreads;
	PFILA2 blockedThreads;
	
	TCB_t* runningThread;
	ucontext_t endThread;

	//flag dispatcher action
};

extern struct control controlBlock;

int cinit(void);
void endThread(void);
int generateTID(void);
int scheduler(void);
int dispatcher(TCB_t *nextRunningThread);

int insertByPrio(PFILA2 pfila, TCB_t *tcb);

#endif
