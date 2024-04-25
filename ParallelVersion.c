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
 *  ...  -> mutex beginLock
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
//mutexes
int CURRENT_DEALER = 0; //who is current dealer this round
pthread_mutex_t beginLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t nextRoundLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t isDealing = PTHREAD_COND_INITIALIZER; //condition to signal that dealer is done
pthread_barrier_t barrier;
int winner = 0;

//deck
int DECK_OF_CARDS[DECK_SIZE];
int playerHand[NUM_THREADS][2] = {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}};
pthread_t PLAYERS[NUM_THREADS];
int topOfDeck = 0;
int bottomOfDeck = 0;




//PROTOTYPES
//player function
void *player(void *arg);
void *beDealer(void *arg);

//helper functions
void initPlayers();
void populateDeck();
void shuffleCards();
void giveCard();
void printDeck();
int drawCard();
void resetHands();
void returnCard(int threadID);

int main(){
    srand(10); //for shuffling the card
    populateDeck();

    pthread_barrier_init(&barrier, NULL, NUM_THREADS);
    initPlayers(); //begins the game

    //joins all threads at the end just in case.
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(PLAYERS[i], NULL);
    }

    return 0;
}

void *player(void *args){ //each player
    int threadID = (int)args;

    for(int roundNum = 1; roundNum <= NUM_ROUNDS; roundNum++){
        //begin game: dealer begins by giving each player 1 card
        if(CURRENT_DEALER == threadID){
            beDealer(args);
            continue;
        }else{
            pthread_cond_wait(&isDealing, &beginLock);
            printf("thread == %d\n", threadID);
            pthread_cond_signal(&isDealing);
        }
        pthread_mutex_unlock(&beginLock);

        pthread_barrier_wait(&barrier); //waiting on dealer to start the game.

        //todo... IMPLEMENT THIS SECTION OF THE CODE
        while(!winner){
            // here: somehow specify which one goes first...

            if(!winner){ //checking to see if the previous one wasn't a winner
                int card = drawCard();
                if(playerHand[threadID][0] == -1) {
                    playerHand[threadID][0] = card;
                }else {
                    if (playerHand[threadID][1] == -1)
                        playerHand[threadID][1] = card;
                }

                if(playerHand[threadID][0] == playerHand[CURRENT_DEALER][0] || playerHand[threadID][1] == playerHand[CURRENT_DEALER][0]){ //WINNER
                    winner = 1;
                    printf("PLAYER %d: wins round %d\n", threadID+1, roundNum);
                }

                returnCard(threadID);

            }else{
                printf("PLAYER %d: lost round %d\n", threadID+1, roundNum);
            }

            //signal for the following one...
        }

        pthread_barrier_wait(&barrier); //wait for next round to start.
    }
    return NULL;
}

void *beDealer(void *args){
    /*
    * First, deal the cards to each player
    */
    int threadID = (int)args;
    pthread_mutex_lock(&beginLock);
    printf("\ndealer :: thread == %d\n", threadID);
    resetHands();
    populateDeck();
    shuffleCards();

    //hand one card to each player
    for(int i = 0; i < NUM_THREADS; i++){
            playerHand[i][0] = drawCard();
    }

    CURRENT_DEALER = (CURRENT_DEALER + 1) % 6;

    for(int i = 0; i < NUM_THREADS; i++){
        printf("i :: %d :: cards handed :: (%d , %d)\n", i, playerHand[i][0], playerHand[i][1]);
    }
    pthread_cond_signal(&isDealing);
    pthread_mutex_unlock(&beginLock);

    pthread_barrier_wait(&barrier); //starting the game

    printf("PLAYER %d: Round ends\n", threadID + 1);

    return NULL;
}

void initPlayers(){
    pthread_create(&PLAYERS[0], NULL, player, (void *) (0));
    pthread_create(&PLAYERS[1], NULL, player, (void *)(1));
    pthread_create(&PLAYERS[2], NULL, player, (void *)(2));
    pthread_create(&PLAYERS[3], NULL, player, (void *)(3));
    pthread_create(&PLAYERS[4], NULL, player, (void *)(4));
    pthread_create(&PLAYERS[5], NULL, player, (void *)(5));
}

void shuffleCards(){
    for(int i = 0; i < DECK_SIZE - 1; i++){
        int j = i + rand() / (RAND_MAX / (DECK_SIZE - i) + 1);
        int t = DECK_OF_CARDS[j];
        DECK_OF_CARDS[j] = DECK_OF_CARDS[i];
        DECK_OF_CARDS[i] = t;
    }
}

int drawCard(){
    int card = DECK_OF_CARDS[topOfDeck];
    DECK_OF_CARDS[topOfDeck] = -1;
    topOfDeck = (topOfDeck + 1) % DECK_SIZE;
    return card;
}

void printHand(int threadID){
    printf("");
}

void returnCard(int playerID){
    int cardToRemove = 1 + rand() % 2;
    printf("random number == %d\n", cardToRemove);
    int cardNum;
    if(cardToRemove == 1){
        cardNum = playerHand[playerID][0];
        playerHand[playerID][0] = -1;
    }else{
        cardNum = playerHand[playerID][1];
        playerHand[playerID][1] = -1;
    }

    DECK_OF_CARDS[bottomOfDeck] = cardNum;
    bottomOfDeck = (bottomOfDeck + 1) % DECK_SIZE;
}

void resetHands(){
    for(int i = 0; i < NUM_THREADS; i++){
        playerHand[i][0] = -1;
        playerHand[i][1] = -1;
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