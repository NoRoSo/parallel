//
// Created by Noe Soto on 4/10/2024.
//

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

#define DECK_SIZE 52

//GLOBAL VARIABLES
// It will be the cards that people choose from.
int DECK_OF_CARDS[DECK_SIZE];

//PROTOTYPES
void populateDeck();
void shuffleCards();
void * printDeck();

int main(){
    srand(time(NULL));


    populateDeck();
    shuffleCards();
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

void * printDeck(){
    for(int i = 0; i < DECK_SIZE; i++){
        printf("Deck at index %d == %d\n", i, DECK_OF_CARDS[i]);
    }
}