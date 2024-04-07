/*
Scrivere un software che simula un sistema di attesa e utilizzo di una serie di risorse.

Abbiamo una stazione con N binari e dei treni in arrivo.

Assunzioni
- N binari e dei treni che devono passare.
- Ogni treno è identificato da un numero intero progressivo.
- Ogni treno arriva dopo un numero di ms random fra (TMin e TMax).
- Ogni treno occupa il binario per un tempo di ms T fisso.

Input
N: Binari
T: millisecondi di permanenza
TMin: Millisecondi minimo prima del prossimo treno
TMax: Millisecondi massimi prima del prossimo treno

Output
Il programma stampa ogni volta che il sistema cambia (un treno entra in stazione o la lascia o si mette in attesa) tre numeri:
- i treni che hanno attraversato,
- i treni che stanno attraversando
- i treni che sono in attesa.

Correttezza
Il software è corretto se il numero di treni che sta attraversando è sempre <= N

Scopo
Effettuare più simulazinoi per dimensionare N.
Trovare il valore di N che per una tripla TMin, TMax, T non produce un aumento indefinito del numero di treni in attesa.

Hint
- L'insieme dei binari potrebbe essere rappresentato da un semaforo
- Ogni treno potrebbe essere un thread
- Per assegnare un ID ad ogni treno potremmo utilizzare una variabile condivisa contatore (0... N) (gestita con un mutex)
- Il numero totale dei treni in attesa e che attraversano (da stampare) potrebbe essere in una variabile condivisa e gestita con un mutex.
- La lista degli ID dei treni che stanno occupando i binari dovrebbe essere condivisa e gestita da un mutex.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

#define NUM_RAILS 4
#define TMin 10
#define TMax 20
#define T 50


static int passed = 0;
static int passing = 0;
static int waiting = 0;

static pthread_mutex_t passed_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t passing_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t waiting_mutex = PTHREAD_MUTEX_INITIALIZER;

void dump();
void queue_train(int train_id);
void train_thread(void* arg);
void pass_on_rail();
void initialize_everything();


void dump(){
    printf("Passed: %3d\tPassing: %3d\tWaiting: %3d\n", passed, passing, waiting);
}

void queue_train(int train_id){

    pthread_mutex_lock(&waiting_mutex);
    waiting++;
    pthread_mutex_unlock(&waiting_mutex);
    dump();
    struct sembuf sem_op = {0, -1, 0};
    int ret = semop(sem_rail, &sem_op, 1);
    if(ret == -1){
        perror("Error waiting on semaphore!");
        exit(-1);
    }
    pthread_mutex_lock(&waiting_mutex);
    waiting--;
    pthread_mutex_unlock(&waiting_mutex);

    pthread_mutex_lock(&passing_mutex);
    passing++;
    pthread_mutex_unlock(&passing_mutex);
    // Critical section
    dump();
    usleep(1000*T);

    pthread_mutex_lock(&passing_mutex);
    passing--;
    pthread_mutex_unlock(&passing_mutex);
    
    dump();

    sem_op.sem_op = 1;
    ret = semop(sem_rail, &sem_op, 1);
    if(ret == -1){
        perror("Error freeing semaphore!");
        exit(-1);
    }
    pthread_mutex_lock(&passed_mutex);
    passed--;
    pthread_mutex_unlock(&passed_mutex);
}

void train_thread(void* arg){
    int train_id = *(int*) arg;

    queue_train(train_id);
    return;
}


int main(int argc, char* argv[]){
    // TODO: Initialize sempahores
    srand(time(NULL));
    semget(sem_id, 1, IPC_PRIVATE);
    int counter = 0;
    pthread_t thread[1000];
    for(int i = 0; i < 1000; i++){
        int next_train_time = rand() % (TMax-TMin+1) + TMin;
        usleep(next_train_time*1000);
        int ret = pthread_create(thread[i], NULL, train_thread, &counter);
        if(ret == -1){
            perror("Error creating thread!");
        }
    }

    for(int i = 0; i < 1000; i++){
        int ret = pthread_join(thread[i]);
        if(ret == -1){
            perror("Error jopining thread!");
        }
    }


}