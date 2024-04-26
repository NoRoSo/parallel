#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define DECK_SIZE 52
#define NUM_THREADS 6
#define NUM_ROUNDS 50000

//GLOBAL VARIABLES
//mutexes
int CURRENT_DEALER = 0; //who is current dealer this round
//pthread_mutex_t beginLock = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t drawCardLock = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t nextRoundLock = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t isDealing = PTHREAD_COND_INITIALIZER; //condition to signal that dealer is done
pthread_barrier_t allPlayers;
pthread_barrier_t justPlayersEnding;
pthread_barrier_t justPlayersBegin;
int winner = 0;
int nextPlayer = 1;

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
void printHand(int threadID);
int drawCard();
void resetGame();
void returnCard(int threadID);

int main(){
    srand(100); //for shuffling the card
    populateDeck();

    //making some barriers I'll be using to make everyone wait.
    pthread_barrier_init(&allPlayers, NULL, NUM_THREADS);
    pthread_barrier_init(&justPlayersEnding, NULL, NUM_THREADS - 1);
    pthread_barrier_init(&justPlayersBegin, NULL, NUM_THREADS - 1);

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
        pthread_barrier_wait(&allPlayers); ///P1
        if(CURRENT_DEALER == threadID){
            printf("\n\n\n\n\n\nROUND == %d\n", roundNum);
            beDealer(args);
            continue;
        }else{
            pthread_barrier_wait(&allPlayers); ///P2
        }


        //todo... IMPLEMENT THIS SECTION OF THE CODE
        while(!winner){
            pthread_barrier_wait(&justPlayersBegin);
            if(!winner) {
                if (nextPlayer == threadID) {
                    printHand(threadID);
                    int card = drawCard();
                    printf("\nPLAYER %d: draws %d\n", threadID+1,card);
                    if (playerHand[threadID][0] == -1) {
                        playerHand[threadID][0] = card;
                    } else {
                        if (playerHand[threadID][1] == -1)
                            playerHand[threadID][1] = card;
                    }

                    if (playerHand[threadID][0] == playerHand[CURRENT_DEALER][0] ||
                        playerHand[threadID][1] == playerHand[CURRENT_DEALER][0]) { //WINNER
                        winner = threadID+1;
                        printHand(threadID);
                        printf(" <> Target card is %d\n", playerHand[CURRENT_DEALER][0]);
                    }else{
                        returnCard(threadID);
                        printHand(threadID);
                        printf("\n");
                        printDeck();
                    }

                    nextPlayer = (nextPlayer + 1) % NUM_THREADS;

                    if(nextPlayer == CURRENT_DEALER)
                        nextPlayer = (nextPlayer + 1) % NUM_THREADS;

                    pthread_barrier_wait(&justPlayersEnding);
                } else {
                    pthread_barrier_wait(&justPlayersEnding);
                }
            }
        }

        if(winner-1 == threadID){
            printf("Player %d: wins round %d\n", threadID+1, roundNum);
            pthread_barrier_wait(&justPlayersEnding); ///P4
        }else{
            pthread_barrier_wait(&justPlayersEnding); ///P4
            printf("Player %d: lost round %d\n", threadID+1, roundNum);
        }


        pthread_barrier_wait(&allPlayers); ///P5 //wait for next round to start.
        pthread_barrier_wait(&allPlayers); ///P6
    }
    return NULL;
}

void *beDealer(void *args){
    /*
    * First, deal the cards to each player
    */
    int threadID = (int)args;
    resetGame();
    populateDeck();
    shuffleCards();

    nextPlayer = (threadID + 1) % NUM_THREADS;

    //hand one card to each player
    for(int i = 0; i < NUM_THREADS; i++){
            playerHand[i][0] = drawCard();
    }
    pthread_barrier_wait(&allPlayers); ///P2 //starting the game AKA done dealing cards



    pthread_barrier_wait(&allPlayers); ///P5 //beginning next round

    CURRENT_DEALER = (CURRENT_DEALER + 1) % NUM_THREADS;
    printf("PLAYER %d: Round ends\n", threadID + 1);

    pthread_barrier_wait(&allPlayers); ///P6
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
    int card1 = playerHand[threadID][0];
    int card2 = playerHand[threadID][1];

    if(card1 != -1 && card2 != -1){
        printf("PLAYER %d: hand (%d, %d)", threadID+1, card1, card2);
    }else{
        if(card1 == -1){
            printf("PLAYER %d: hand %d", threadID+1, card2);
        }else{
            printf("PLAYER %d: hand %d", threadID+1, card1);
        }
    }
}

void returnCard(int playerID){
    int cardToRemove = 1 + rand() % 2;
    int cardNum;
    if(cardToRemove == 1){
        cardNum = playerHand[playerID][0];
        playerHand[playerID][0] = -1;
    }else{
        cardNum = playerHand[playerID][1];
        playerHand[playerID][1] = -1;
    }

    printf("PLAYER %d: discards %d at random\n", playerID+1, cardNum);

    DECK_OF_CARDS[bottomOfDeck] = cardNum;
    bottomOfDeck = (bottomOfDeck + 1) % DECK_SIZE;
}

void resetGame(){
    for(int i = 0; i < NUM_THREADS; i++){
        playerHand[i][0] = -1;
        playerHand[i][1] = -1;
    }
    winner = 0;
    topOfDeck = 0;
    bottomOfDeck = 0;
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
    printf("DECK:");
    for(int i = 0; i < DECK_SIZE; i++){
        printf(" %d", DECK_OF_CARDS[i]);
    }
    printf("\n");
}