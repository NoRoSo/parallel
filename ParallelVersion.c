/*
 * pthread_t is a type to get the ID
 *
 * attr doesn't need to be changed, set to NULL, however
 * it could be used to create a priority of all threads.
 *
 * void *(*start_routine)(void*) is the address of a function
 *
 * void *restrict arg is any params you wanna pass to the function.
 */

/*
 * Condition Variables:
 *
 *  - Make a thread wait until a condition is met.
 *
 *  T1:
 *  ...  -> mutex lock
 *  pthread_cond_wait(&cv , &mutex); as you wait pthreads lib unlocks it.
 *
 *  T2:
 *  pthread_mutex_lock();
 *  pthread_cond_signal();
 *  pthread_mutex_unlock();
 *
 *
 */

/*
 * func thread_i:
 *
 * if(currentDealer == threadID (which starts at 1 because player 1 is first to deal)
 *      shuffle();
 *      deal();
 *      currentDealer++;
 *      signal(1);
 *
 * everyone else waits();
 *
 *
 */

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

#include <unistd.h>

#define DECK_SIZE 52
#define NUM_THREADS 6
#define NUM_ROUNDS 1

//GLOBAL VARIABLES
pthread_mutex_t CURRENT_DEALER = PTHREAD_MUTEX_INITIALIZER; //mutex for current dealer
pthread_cond_t isDealing = PTHREAD_COND_INITIALIZER; //condition to signal that dealer is done
int DECK_OF_CARDS[DECK_SIZE];
pthread_t PLAYERS[NUM_THREADS];


//PROTOTYPES
//player function
void *player(void *arg);

//helper functions
void initPlayers();
void populateDeck();
void shuffleCards();
void printDeck();

int main(){
    srand(time(NULL)); //for shuffling the card
    populateDeck();

    for(int game = 1; game <= NUM_ROUNDS; game++){
        initPlayers();

        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(PLAYERS[i], NULL);
        }
    }

    pthread_exit(NULL);

    return 0;
}

void *player(void *args){ //each player
    int threadID = (int)args;

    pthread_mutex_lock(&CURRENT_DEALER);
    while(1){
        if(CURRENT_DEALER == threadID){
            printf("\nthread == %d :: ", threadID);
            //usleep(100);
            shuffleCards();
            printDeck();
//            CURRENT_DEALER = (CURRENT_DEALER % NUM_THREADS) + 1;
//            pthread_cond_broadcast(&isDealing);
            pthread_mutex_unlock(&CURRENT_DEALER);
        }else{
            pthread_mutex_lock(&CURRENT_DEALER);
            printf("thread == %d\n", threadID);
        }

        printf("hello == %d\n", threadID);

        return NULL;
    }
}

void initPlayers(){
    pthread_create(&PLAYERS[0], NULL, player, (void *) (1));
    pthread_create(&PLAYERS[1], NULL, player, (void *)(2));
    pthread_create(&PLAYERS[2], NULL, player, (void *)(3));
    pthread_create(&PLAYERS[3], NULL, player, (void *)(4));
    pthread_create(&PLAYERS[4], NULL, player, (void *)(5));
    pthread_create(&PLAYERS[5], NULL, player, (void *)(6));
}

void shuffleCards(){
    for(int i = 0; i < DECK_SIZE - 1; i++){
        int j = i + rand() / (RAND_MAX / (DECK_SIZE - i) + 1);
        int t = DECK_OF_CARDS[j];
        DECK_OF_CARDS[j] = DECK_OF_CARDS[i];
        DECK_OF_CARDS[i] = t;
    }
}

void populateDeck(){
    int index = 0;
    for(int i = 0; i < 4; i++){
        for(int x = 1; x <= 13; x++){
            DECK_OF_CARDS[index++] = x;
        }
    }
}

void printDeck(){
    for(int i = 0; i < DECK_SIZE; i++){
        printf("%d ", DECK_OF_CARDS[i]);
    }
    printf("\n");
}