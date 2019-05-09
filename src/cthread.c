#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"


#define PRIORIDADE_ALTA 0
#define PRIORIDADE_MEDIA 1
#define PRIORIDADE_BAIXA 2
#define MAIN_TID 0

#define INICIALIZADA 1
#define TRUE 1
#define FALSE 0
int cthread_inicializada = 0;
int id_atual = 0;


PFILA2 filaDePrioridadeAlta, filaDePrioridadeMedia, filaDePrioridadeBaixa;
FILA2 bloqueado;

ucontext_t *executando = NULL, *init = NULL, *termino = NULL;
TCB_t *MAIN;



int cthreadJaFoiInicializada(){
    if(cthread_inicializada == INICIALIZADA)
        return TRUE;
    return FALSE;
}

void novoTID(){
    return id_atual++;
}

TCB_t* criarTCB(int prioridade){

    TCB_t *novoTCB = (TCB_t*) malloc(sizeof(TCB_t));
    if(novoTCB == NULL){
        return NULL;
    }
    novoTCB->tid = novoTID();
    novoTCB->prio = prioridade;
    novoTCB->state = PROCST_APTO;
    return novoTCB;
}

void criarContexto(ucontext_t *contexto, void* (*start)(void*), void *arg, ucontext_t* link){

    char *pilha = (char*) malloc(SIGSTKSZ);
    if(pilha == NULL ){
        contexto = NULL;
        return;
    }
    contexto = (ucontext_t*) malloc(sizeof(ucontext_t));
    if(contexto == NULL ){
        contexto = NULL;
        return;
    }
    makeContext(novoContexto, start, 1, void *arg);
    contexto->uc_stack.ss_sp = pilha;
    contexto->uc_stack.ss_size = SIGSTKSZ;
    contexto->uc_link = link;
}

#define ERRO_AO_INICIALIZAR     -1
#define SUCESSO_AO_INICIALIZAR   0
int inicializar(){

    criarContexto(init, escalonador, void, NULL);
    criarContexto(termino, finalizar, void, NULL);
    MAIN = (TCB_t*) malloc(sizeof(TCB_t));

    if(init == NULL || termino == NULL || MAIN == NULL){
        return ERRO_AO_INICIALIZAR;
    }

    MAIN->state = PROCST_EXEC;
    MAIN->tid = MAIN_TID;
    MAIN->prio = PRIORIDADE_BAIXA;

    return SUCESSO_AO_INICIALIZAR_INIT;
}

void finalizar(){
    TCB_t tcb_atual;
    getcontext(&atual);
        //liberar threads q estavam aguardando o fim dessa execucao
    free(atual.context);
    free(atual);
    setcontext(init);
}


#define MSG_ERRO_CRITICO  "ERRO CRITÍCO"
void escalonador(){
    executando = proximoProcessoDaFila();
    if(executando == NULL){
        printf(MSG_ERRO_CRITICO);
        return;
    }
    executando->state = PROCST_EXEC;
    setcontext(&(executando->context));
}















#define    ERRO_AO_ADICIONAR_NA_FILA_APTO -1
int adicionarNaFila(TCB_t *tcb){

    PNODE2 novoNo = (PNODE2) malloc(sizeof(NODE2));
    novoNo->node = tcb;
    if(tcb == NULL || novoNo== NULL){
        return ERRO_AO_ADICIONAR_NA_FILA_APTO;
    }

    int status;

    switch(tcb->prio){
        case PRIORIDADE_ALTA:
            status =  AppendFila2(filaDePrioridadeAlta, novoNo);
            break;
        case PRIORIDADE_MEDIA:
            status = AppendFila2(filaDePrioridadeMedia, novoNo);
            break;
        case PRIORIDADE_BAIXA:
            status =  AppendFila2(filaDePrioridadeBaixa, novoNo);
            break;
        case default:
            status  ERRO_AO_ADICIONAR_NA_FILA_APTO;
    }

    return status;
}

TCB_t* proximoProcessoDaFila(){
    TCB_t *proximoProcesso;

    if(FirstFila2(filaDePrioridadeAlta) == 0){
        proximoProcesso = GetAtIteratorFila2(filaDePrioridadeAlta)->node;
        DeleteAtIteratorFila2(filaDePrioridadeAlta);
        return proximoProcesso;
    }
    if(FirstFila2(filaDePrioridadeMedia) == 0){
        proximoProcesso = GetAtIteratorFila2(filaDePrioridadeMedia)->node;
        DeleteAtIteratorFila2(filaDePrioridadeMedia);
        return proximoProcesso;
    }
    if(FirstFila2(filaDePrioridadeBaixa) == 0){
        proximoProcesso = GetAtIteratorFila2(filaDePrioridadeBaixa)->node;
        DeleteAtIteratorFila2(filaDePrioridadeBaixa);
        return proximoProcesso;
    }

    return NULL;

}




























#define ERRO_AO_CRIAR_TCB -1
int ccreate (void* (*start)(void*), void *arg, int prio) {
    if(cthreadJaFoiInicializada == FALSE){
        inicializar();
    }

    TCB_t *novoTCB;
    novoTCB = criarTCB(prio);
    criarContexto(&(novoTCB->context),start, arg, termino);

    if(novoTCB == NULL || &(novoTCB->context) == NULL){
        return ERRO_AO_CRIAR_TCB;
    }
    adicionarNaFila(novoTCB);
    return novoTCB->prio;
}


#define ERRO_AO_MUDAR_PRIORIDADE    -1
#define SUCESSO_AO_MUDAR_PRIORIDADE  0
int csetprio(int tid, int prio) {
    if(cthreadJaFoiInicializada == FALSE){
        inicializar();
    }
    if(prio <=3 && prio>=0) {
        executando->prio = prio;
        return SUCESSO_AO_MUDAR_PRIORIDADE;
    }
    return ERRO_AO_MUDAR_PRIORIDADE;
}

int cyield(void) {
    if(cthreadJaFoiInicializada == FALSE){
        inicializar();
    }
    return -1;
}

int cjoin(int tid) {
    if(cthreadJaFoiInicializada == FALSE){
        inicializar();
    }
    return -1;
}

