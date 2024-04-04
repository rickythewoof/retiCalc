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
#include <semaphore.h>
#include <pthread.h>

#define NUM_BINARI 4
#define TMin 10
#define TMax 20
#define occupy_time 50

typedef struct Train{
    int id;
} Train;

void dump();
void queue_train();
void train_thread();

int main(int argc, char* argv[]){

}