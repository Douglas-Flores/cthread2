#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"

#include <signal.h>

#define STACK_SIZE SIGSTKSZ

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
PFILA2 bloqueado, join;

ucontext_t *init = NULL, *termino = NULL;
TCB_t *MAIN, *executando;



int cthreadJaFoiInicializada(){
    if(cthread_inicializada == 1){
        return TRUE;
    }
    return FALSE;
}

int novoTID(){
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

void criarContexto(ucontext_t *contexto, void* (start), void *arg, ucontext_t* link){

    char *pilha = (char*) malloc(STACK_SIZE);
    if(pilha == NULL ){
        contexto = NULL;
        return;
    }
    contexto = (ucontext_t*) malloc(sizeof(ucontext_t));
    if(contexto == NULL ){
        contexto = NULL;
        return;
    }
    makecontext(contexto, (void *)start, 1, arg);
    contexto->uc_stack.ss_sp = pilha;
    contexto->uc_stack.ss_size = STACK_SIZE;
    contexto->uc_link = link;
}


TCB_t* proximoProcessoDaFila(){
    TCB_t *proximoProcesso;
    PNODE2 temp;
    if(FirstFila2(filaDePrioridadeAlta) == 0){
        temp = GetAtIteratorFila2(filaDePrioridadeAlta);
        proximoProcesso = temp->node;
        DeleteAtIteratorFila2(filaDePrioridadeAlta);
        return proximoProcesso;
    }
    if(FirstFila2(filaDePrioridadeMedia) == 0){
        temp = GetAtIteratorFila2(filaDePrioridadeMedia);
        proximoProcesso = temp->node;
        DeleteAtIteratorFila2(filaDePrioridadeMedia);
        return proximoProcesso;
    }
    if(FirstFila2(filaDePrioridadeBaixa) == 0){
        temp = GetAtIteratorFila2(filaDePrioridadeBaixa);
        proximoProcesso = temp->node;
        DeleteAtIteratorFila2(filaDePrioridadeBaixa);
        return proximoProcesso;
    }

    return NULL;

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

int adicionarNaFila(TCB_t *tcb){

    PNODE2 novoNo = (PNODE2) malloc(sizeof(NODE2));
    novoNo->node = tcb;
    if(tcb == NULL || novoNo== NULL){
        return -1;
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
        default:
            status =  -1;
    }

    return status;
}

void desbloquear(int tid){
    int sucesso = FirstFila2(bloqueado);
    if(sucesso != 0){
        return;
    }
    do{
        PNODE2  noAtual = GetAtIteratorFila2(join);
        TCB_t *tcb_atual = noAtual->node;
        if(tcb_atual->tid == tid){
            adicionarNaFila(tcb_atual);
            DeleteAtIteratorFila2(bloqueado);
            return;
        }
        sucesso = NextFila2(bloqueado);
    }while(sucesso == 0);
}

int jaExisteJoinDesseTID(int tid){
    int sucesso = FirstFila2(join);
    if(sucesso != 0){
        return FALSE;
    }
    do{
        PNODE2  noAtual = GetAtIteratorFila2(join);
        Dependencia *dependencia_atual = noAtual->node;
        if(dependencia_atual->tid_esperada == tid){
            return TRUE;
        }
        sucesso = NextFila2(join);
    } while(sucesso == 0);

    return FALSE;
}

int estaSendoEsperada(int tid){
    int sucesso = FirstFila2(join);
    if(sucesso != 0){
        return FALSE;
    }
    do{
        PNODE2  noAtual = GetAtIteratorFila2(join);
        Dependencia *dependencia_atual = noAtual->node;
        if(dependencia_atual->tid_esperada == tid){
            return TRUE;
        }
        sucesso = NextFila2(join);
    }while(sucesso == 0);
    return FALSE;
}

void terminaJoin(int tid){
    int sucesso = FirstFila2(join);
    if(sucesso != 0){
        return;
    }
    do{
        PNODE2  noAtual = GetAtIteratorFila2(join);
        Dependencia *dependencia_atual = noAtual->node;
        if(dependencia_atual->tid_esperada  == tid){
            desbloquear(dependencia_atual->tid_esperando);
            DeleteAtIteratorFila2(join);
            return;
        }
        sucesso = NextFila2(join);
    }while(sucesso == 0);
}



void finalizar(){
    TCB_t *tcb_atual = executando;
    getcontext(&(tcb_atual->context));

    if(estaSendoEsperada(tcb_atual->tid)){
        terminaJoin(tcb_atual->tid);
    }

    free(&(tcb_atual->context));
    free(tcb_atual);
    setcontext(init);
}

#define ERRO_AO_INICIALIZAR     -1
#define SUCESSO_AO_INICIALIZAR   0
int inicializar(){
    cthread_inicializada = INICIALIZADA;

    criarContexto(init, escalonador, NULL, NULL);
    criarContexto(termino, finalizar, NULL, NULL);
    MAIN = (TCB_t*) malloc(sizeof(TCB_t));

    if(init == NULL || termino == NULL || MAIN == NULL){
        return ERRO_AO_INICIALIZAR;
    }

    CreateFila2(filaDePrioridadeAlta);
    CreateFila2(filaDePrioridadeMedia);
    CreateFila2(filaDePrioridadeBaixa);
    CreateFila2(bloqueado);
    CreateFila2(join);



    MAIN->state = PROCST_EXEC;
    MAIN->tid = MAIN_TID;
    MAIN->prio = PRIORIDADE_BAIXA;

    return SUCESSO_AO_INICIALIZAR;
}

#define ERRO_AO_CRIAR_TCB -1
int ccreate (void* (*start)(void*), void *arg, int prio) {
    if(cthreadJaFoiInicializada() == FALSE){
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
    if(cthreadJaFoiInicializada() == FALSE){
        inicializar();
    }
    if(prio <=3 && prio>=0) {
        executando->prio = prio;
        return SUCESSO_AO_MUDAR_PRIORIDADE;
    }
    return ERRO_AO_MUDAR_PRIORIDADE;
}

int cyield(void) {
    TCB_t *atual = executando;
    if(cthreadJaFoiInicializada() == FALSE){
        inicializar();
    }
    if(adicionarNaFila(atual)!=0){
        return -1;
    }
    TCB_t* novoTCB = proximoProcessoDaFila();
    return swapcontext(&(atual->context), &(novoTCB->context));
}

int cjoin(int tid) {
    if(cthreadJaFoiInicializada() == FALSE){
        inicializar();
    }
    if(jaExisteJoinDesseTID(tid)){
        return -1;
    }
    PNODE2 novoNo = (PNODE2) malloc(sizeof(NODE2));
    Dependencia *novaDependencia = (Dependencia*) malloc(sizeof(Dependencia));
    if(novaDependencia == NULL){
        return -1;
    }
    novoNo->node = novaDependencia;
    novaDependencia->tid_esperada = tid;
    novaDependencia->tid_esperando = executando->tid;
    return 0;
}


int csem_init(csem_t *sem, int count) {
    //Cria a fila do semáforo
    FILA2 wait;
    //Inicializa a fila
    if(CreateFila2(&wait) != 0)
        return -1;
    //ponteiro para fila de bloqueados (wait)
    sem->fila = &wait;
    //inicialização do count
    sem->count = count;

    return 0;
}

int cwait(csem_t *sem) {
    if ( sem->count <= 0 ){
        //estado passa a ser bloqueado
        TCB_t *atual = executando;
        atual->state = PROCST_BLOQ;
        //adiciona na fila de bloqueados
        if(AppendFila2(sem->fila, atual) != 0)
            return -1;
        //decrementa o count
        sem->count = sem->count - 1;
        executando = proximoProcessoDaFila();
        swapcontext(&(atual->context), &(executando->context));
        //retorna com sucesso
        return 0;
    }
    else{
        sem->count = sem->count - 1;
        return 0;
    }
}

int csignal(csem_t *sem) {
    sem->count = sem->count + 1;
    //Coloca o iterador no primeiro da fila
    if(FirstFila2(sem->fila) != 0){
        return 0;
    }

    TCB_t *temp;
    temp = ((PNODE2) GetAtIteratorFila2(sem->fila))->node;
    if(temp == NULL){
        return -1;
    }
    //seta o nodo para apto
    temp->state = PROCST_APTO;
    adicionarNaFila(temp);
    //remove da fila de espera
    return DeleteAtIteratorFila2(sem->fila);
}

int cidentify (char *name, int size) {
    strncpy (name, "DOUGLAS SOUZA FLÔRES - 262524\nRenan Ferigatto - 260845\nRodrigo Costa Machado - 260849", size);
    return 0;
}




