
#include <stdio.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"


#define PRIORIDADE_ALTA 0
#define PRIORIDADE_MEDIA 1
#define PRIORIDADE_BAIXA 2
#define MAIN_TID 0

//Filas
FILA2 filaAlta, filaMedia, filaBaixa;
FILA2 join, wait;

TCB_t *executando;
TCB_t Main;
int id_atual = 0;

void novoId(){
    return id_atual++;
}

/*
 * Aloca memória para todas as filas
 * */
void inicializarFilas(){
    &filaAlta = (FILA2) malloc(sizeof(FILA2));
    filaAlta.first = NULL;
    filaAlta.last = NULL;
    filaAlta.it = NULL;

    &filaMedia = (FILA2) malloc(sizeof(FILA2));
    filaMedia.first = NULL;
    filaMedia.last = NULL;
    filaMedia.it = NULL;

    &filaBaixa = (FILA2) malloc(sizeof(FILA2));
    filaBaixa.first = NULL;
    filaBaixa.last = NULL;
    filaBaixa.it = NULL;

    &join = (FILA2) malloc(sizeof(FILA2));
    join.first = NULL;
    join.last = NULL;
    join.it = NULL;

    &wait= (FILA2) malloc(sizeof(FILA2));
    wait.first = NULL;
    wait.last = NULL;
    wait.it = NULL;
}


/*
 * Cria TCB Main para o processo que for utilizar a biblioteca
 * Retorno: ponteiro para o TCB criado
 * */
void criarMain(){
    &Main = (TCB_t) malloc(sizeof(TCB_t));
    Main.state = PROCST_APTO;
    Main.prio = PRIORIDADE_BAIXA;
    Main.tid = 0;
    getcontext(&(Main.context));
}


/*
 * De acordo com a prioridade do tcb, adiciona na fila correspondente
 * Retorno:
 * caso ==0 sucesso
 * caso !=0 erro
 * */
int appendTCBNaFilaCorrespondenteAPrioridade(TCB_t *tcb){
    int status;
    switch (tcb->prio)
    {
        case PRIORIDADE_ALTA:
            status = AppendFila2(&filaAlta, tcb);
            break;
        case PRIORIDADE_MEDIA:
            status = AppendFila2(&filaMedia, tcb);
            break;
        case PRIORIDADE_BAIXA:
            status = AppendFila2(&filaBaixa, tcb);
            break;
    }
    return status;
}

/*
 * Inicializa as Filas, cria o TCB para a main
 * e coloca a Main em executando
 * */
int inicializarScheduler(){
    inicializarFilas();
    criarMain();
}

/*
 * Procura pelo primeiro TCB de cada fila, indo da de maior prioridade
 * até a de menor
 * Retorno:
 * TCB_t* da próxima thread a ser executada ou null em caso de não haver mais threads
 * */
TCB_t* proximoProcessoASerExecutado(){
    if(filaAlta.first != NULL){
        return filaAlta.first;
    }
    else if(filaMedia.first != NULL){
        return filaMedia.first;
    }
    else if(filaBaixa.first != NULL){
        return filaBaixa.first;
    }
    else
        return NULL;
}


int ccreate (void* (*start)(void*), void *arg, int prio) {
    TCB_t tcb;
    tcb.prio = prio;
    tcb.tid = novoId();
    tcb.state = PROCST_APTO;
    makecontext(&(tcb.context), start,1, arg);
	return -1;
}

int csetprio(int tid, int prio) {
    //checar se nova prioridade esta dentro do intervalo
    executando->prio = prio;
	return -1;
}

int cyield(void) {
    //salva contexto atual
    getcontext(&(executando->context));
    //adiciona tcb na fila de apto
    appendTCBNaFilaCorrespondenteAPrioridade(executando->prio);
    //faz a troca de contexto
    swapcontext(executando->context, proximoProcessoASerExecutado()->context);
	return -1;
}

int cjoin(int tid) {
	return -1;
}

int csem_init(csem_t *sem, int count) {
	return -1;
}

int cwait(csem_t *sem) {
	return -1;
}

int csignal(csem_t *sem) {
	return -1;
}

int cidentify (char *name, int size) {
	strncpy (name, "DOUGLAS SOUZA FLÔRES - 262524\nRenan Ferigatto - 260845\nRodrigo Costa Machado - 260849", size);
	return 0;
}


