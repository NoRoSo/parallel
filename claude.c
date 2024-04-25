#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

//#define NUM_PLAYERS 6
//#define NUM_ROUNDS 6
//#define DECK_SIZE 52

// Card values
char *card_values[] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
#define NUM_VALUES 13

// Shared resources
int NUM_PLAYERS = 6;
int NUM_ROUNDS = 6;
int DECK_SIZE = 52;

int deck[52];
int target_card;
int current_round = 0;
int current_player = 0;
FILE *log_file;
pthread_mutex_t lock;

// Function prototypes
void shuffle_deck(int deck[], int seed);
int draw_card(int deck[], int *deck_size);
void discard_card(int deck[], int *deck_size, int card);
void *player_thread(void *arg);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }

    int seed = atoi(argv[1]);
    srand(seed);

    // Initialize deck
    for (int i = 0; i < DECK_SIZE; i++) {
        deck[i] = i % NUM_VALUES;
    }

    // Initialize log file
    log_file = fopen("log.txt", "w");
    if (log_file == NULL) {
        printf("Error opening log file\n");
        return 1;
    }

    // Initialize mutex
    pthread_mutex_init(&lock, NULL);

    // Create player threads
    pthread_t player_threads[NUM_PLAYERS];
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (pthread_create(&player_threads[i], NULL, player_thread, (void *)(long)i) != 0) {
            printf("Error creating player thread\n");
            return 1;
        }
    }

    // Wait for all player threads to finish
    for (int i = 0; i < NUM_PLAYERS; i++) {
        pthread_join(player_threads[i], NULL);
    }

    // Clean up
    fclose(log_file);
    pthread_mutex_destroy(&lock);

    return 0;
}

void shuffle_deck(int deck[], int seed) {
    srand(seed);
    for (int i = DECK_SIZE - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

int draw_card(int deck[], int *deck_size) {
    int card = deck[0];
    for (int i = 0; i < *deck_size - 1; i++) {
        deck[i] = deck[i + 1];
    }
    (*deck_size)--;
    return card;
}

void discard_card(int deck[], int *deck_size, int card) {
    deck[*deck_size] = card;
    (*deck_size)++;
}

void *player_thread(void *arg) {
    int player_id = (long)arg;

    for (int round = 0; round < NUM_ROUNDS; round++) {
        // Wait for the current round to start
        while (current_round != round) {
            usleep(100000); // Sleep for 100ms
        }

        // If this player is the dealer, shuffle the deck and draw the target card
        if (player_id == current_player) {
            shuffle_deck(deck, rand());
            pthread_mutex_lock(&lock);
            target_card = draw_card(deck, &DECK_SIZE);
            pthread_mutex_unlock(&lock);
            fprintf(log_file, "PLAYER %d: Target Card %s\n", player_id + 1, card_values[target_card]);
        }

        // Wait for all players to receive their initial card
        while (current_player != (player_id + 1) % NUM_PLAYERS) {
            usleep(100000); // Sleep for 100ms
        }

        // Draw initial card
        pthread_mutex_lock(&lock);
        int player_hand[2] = {draw_card(deck, &DECK_SIZE), -1};
        pthread_mutex_unlock(&lock);
        fprintf(log_file, "PLAYER %d: hand %s\n", player_id + 1, card_values[player_hand[0]]);

        // Play the round
        int winner = -1;
        while (winner == -1) {
            // Wait for this player's turn
            while (current_player != player_id) {
                usleep(100000); // Sleep for 100ms
            }

            // Draw a card
            pthread_mutex_lock(&lock);
            player_hand[1] = draw_card(deck, &DECK_SIZE);
            pthread_mutex_unlock(&lock);
            fprintf(log_file, "PLAYER %d: draws %s\n", player_id + 1, card_values[player_hand[1]]);

            // Check for a match
            if (player_hand[0] == target_card || player_hand[1] == target_card) {
                winner = player_id;
                fprintf(log_file, "PLAYER %d: hand (%s,%s) <> Target card is %s\n", player_id + 1,
                        card_values[player_hand[0]], card_values[player_hand[1]], card_values[target_card]);
                fprintf(log_file, "PLAYER %d: wins round %d\n", player_id + 1, round + 1);
            } else {
                // Discard a random card
                int discard_index = rand() % 2;
                pthread_mutex_lock(&lock);
                discard_card(deck, &DECK_SIZE, player_hand[discard_index]);
                pthread_mutex_unlock(&lock);
                fprintf(log_file, "PLAYER %d: discards %s at random\n", player_id + 1, card_values[player_hand[discard_index]]);
                player_hand[discard_index] = -1;
            }

            // Move to the next player
            current_player = (current_player + 1) % NUM_PLAYERS;
        }

        // Print the result for this round
        for (int i = 0; i < NUM_PLAYERS; i++) {
            if (i == winner) {
                fprintf(log_file, "PLAYER %d: won round %d\n", i + 1, round + 1);
            } else {
                printf("PLAYER %d: lost round %d\n", i + 1, round + 1);
            }
        }

        // Move to the next round
        current_round++;
        current_player = (current_player + 1) % NUM_PLAYERS;
    }

    return NULL;
}