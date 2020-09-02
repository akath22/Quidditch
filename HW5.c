#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

// Helpers
void random_sleep(int);

// Thread functions
void *bludger_quaffle_beater_function(void *);
void *snitch_function(void *);
void *chaser_function(void *);
void *keeper_function(void *);
void *seeker_function(void *);
void *goal_function(void *);

// Signal handlers
void hit_by_bludger_or_goal_attempt(int);
void saved_by_beater_or_goal_blocked(int);
void caught_quaffle(int);


// Player representation
struct Player
{
    pthread_t *thread;
    char team;
    char name[11];
    int id;
    char state[54];
};

// Team A
pthread_t seeker_a;
pthread_t keeper_a;
pthread_t beater_a_1, beater_a_2;
pthread_t chaser_a_1, chaser_a_2, chaser_a_3;

// Team B
pthread_t seeker_b;
pthread_t keeper_b;
pthread_t beater_b_1, beater_b_2;
pthread_t chaser_b_1, chaser_b_2, chaser_b_3;

// Player structs
struct Player seeker_a_struct = {.thread = &seeker_a, .team = 'A', .name = "seeker_a", .id = 1, .state= "Playing"};
struct Player seeker_b_struct = {.thread = &seeker_b, .team = 'B', .name = "seeker_b", .id = 2, .state= "Playing"};
struct Player keeper_a_struct = {.thread = &keeper_a, .team = 'A', .name = "keeper_a", .id = 3, .state= "Playing"};
struct Player keeper_b_struct = {.thread = &keeper_b, .team = 'B', .name = "keeper_b", .id = 4, .state= "Playing"};
struct Player beater_a_1_struct = {.thread = &beater_a_1, .team = 'A', .name = "beater_a_1", .id = 5,.state= "Playing"};
struct Player beater_a_2_struct = {.thread = &beater_a_2, .team = 'A', .name = "beater_a_2", .id = 6, .state= "Playing"};
struct Player beater_b_1_struct = {.thread = &beater_b_1, .team = 'B', .name = "beater_b_1", .id = 7, .state= "Playing"};
struct Player beater_b_2_struct = {.thread = &beater_b_2, .team = 'B', .name = "beater_b_2", .id = 8, .state= "Playing"};
struct Player chaser_a_1_struct = {.thread = &chaser_a_1, .team = 'A', .name = "chaser_a_1", .id = 9, .state= "Playing"};
struct Player chaser_a_2_struct = {.thread = &chaser_a_2, .team = 'A', .name = "chaser_a_2", .id = 10, .state= "Playing"};
struct Player chaser_a_3_struct = {.thread = &chaser_a_3, .team = 'A', .name = "chaser_a_3", .id = 11, .state= "Playing"};
struct Player chaser_b_1_struct = {.thread = &chaser_b_1, .team = 'B', .name = "chaser_b_1", .id = 12, .state= "Playing"};
struct Player chaser_b_2_struct = {.thread = &chaser_b_2, .team = 'B', .name = "chaser_b_2", .id = 13, .state= "Playing"};
struct Player chaser_b_3_struct = {.thread = &chaser_b_3, .team = 'B', .name = "chaser_b_3", .id = 14, .state= "Playing"};

// Array of pointers to all of the player structs
struct Player *player_pointers[14] = {&seeker_a_struct, &seeker_b_struct,
                                      &keeper_a_struct, &keeper_b_struct,
                                      &beater_a_1_struct, &beater_a_2_struct, &beater_b_1_struct, &beater_b_2_struct,
                                      &chaser_a_1_struct, &chaser_a_2_struct, &chaser_a_3_struct, &chaser_b_1_struct, &chaser_b_2_struct, &chaser_b_3_struct};

// Balls
pthread_t snitch;
pthread_t quaffle;
pthread_t bludger_1, bludger_2;



// Goals
pthread_t goal_a, goal_b;

//Goal_A=0 & Goal_B=0 represents that Goal is safe..
//Goal_A=1 & Goal_B=1 represents that Chaser is trying to score a goal... 
static volatile sig_atomic_t Goal_A = 0;
static volatile sig_atomic_t Goal_B = 0;

// Global variables
const int MAX_BLUDGER_SLEEP_TIME = 8;
const int MAX_SNITCH_SLEEP_TIME = 10;
const int MAX_SEEKER_SLEEP_TIME = 20;
const int MAX_KEEPER_SLEEP_TIME = 15;
const int MIN_SLEEP_TIME = 1;
const int NUMBER_OF_PLAYERS = 14;
const int NUMBER_OF_CHASERS = 6;
static volatile sig_atomic_t Caught_Snitch = 0;
static volatile sig_atomic_t team_a_score = 0;
static volatile sig_atomic_t team_b_score = 0;


int main()
{
    srand((unsigned int)time(NULL));

    // Setup signal handlers
    struct sigaction sigint_act;
    sigint_act.sa_handler = hit_by_bludger_or_goal_attempt;
    sigint_act.sa_flags = 0;
    sigemptyset(&sigint_act.sa_mask);
    sigaction(SIGINT, &sigint_act, NULL);

    struct sigaction sigusr1_act;
    sigusr1_act.sa_handler = saved_by_beater_or_goal_blocked;
    sigusr1_act.sa_flags = 0;
    sigemptyset(&sigusr1_act.sa_mask);
    sigaction(SIGUSR1, &sigusr1_act, NULL);

    struct sigaction sigusr2_act;
    sigusr2_act.sa_handler = caught_quaffle;
    sigusr2_act.sa_flags = 0;
    sigemptyset(&sigusr2_act.sa_mask);
    sigaction(SIGUSR2, &sigusr2_act, NULL);

    // Create bludgers
    char b = 'b';
    pthread_create(&bludger_1, NULL, &bludger_quaffle_beater_function, (void *)&b);
    pthread_create(&bludger_2, NULL, &bludger_quaffle_beater_function, (void *)&b);

    // Create quaffles
    char q = 'q';
    pthread_create(&quaffle, NULL, &bludger_quaffle_beater_function, (void *)&q);

    // Create snitch
    pthread_create(&snitch, NULL, &snitch_function, NULL);

    // Create chasers
    pthread_create(&chaser_a_1, NULL, &chaser_function, NULL);
    pthread_create(&chaser_a_2, NULL, &chaser_function, NULL);
    pthread_create(&chaser_a_3, NULL, &chaser_function, NULL);
    pthread_create(&chaser_b_1, NULL, &chaser_function, NULL);
    pthread_create(&chaser_b_2, NULL, &chaser_function, NULL);
    pthread_create(&chaser_b_3, NULL, &chaser_function, NULL);

    // Create keepers
    pthread_create(&keeper_a, NULL, &keeper_function, NULL);
    pthread_create(&keeper_b, NULL, &keeper_function, NULL);

    // Create beaters
    char t = 't';
    pthread_create(&beater_a_1, NULL, &bludger_quaffle_beater_function, (void *)&t);
    pthread_create(&beater_a_2, NULL, &bludger_quaffle_beater_function, (void *)&t);
    pthread_create(&beater_b_1, NULL, &bludger_quaffle_beater_function, (void *)&t);
    pthread_create(&beater_b_2, NULL, &bludger_quaffle_beater_function, (void *)&t);

    // Create seekers
    pthread_create(&seeker_a, NULL, &seeker_function, NULL);
    pthread_create(&seeker_b, NULL, &seeker_function, NULL);

    // Create goals
    pthread_create(&goal_a, NULL, &goal_function, NULL);
    pthread_create(&goal_b, NULL, &goal_function, NULL);
    
    printf("Welcome to Quidditch Game! \n"); 
    printf("Let the game begin!\n");
    // Join all players, so the game exits if they all fall off their brooms
    for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
    {
        pthread_join(*player_pointers[i]->thread, NULL);
    }

    printf("All players have been bludgend. GAME OVER...\n");
    exit(0);
}

// Thread functions
void *bludger_quaffle_beater_function(void *p)
{
    int num = NUMBER_OF_PLAYERS;
    int offset = 0;
    int signum = SIGINT;

    // Determine which type of entity that this thread represents
    if (*((char *)p) == 'b')
    {
        // case for bludger
        // variables already apply
    }
    else if (*((char *)p) == 'q')
    {
        // case for quaffle
        num = NUMBER_OF_CHASERS;
        offset = 8;
        signum = SIGUSR2;
    }
    else if (*((char *)p) == 't')
    {
        // case for beater
        signum = SIGUSR1;
    }

    // Sleep, pick random player, send signal according to this threads type, repeat
    for (;;)
    {
        if (*((char *)p) == 't')
             random_sleep(2);
        else
             random_sleep(MAX_BLUDGER_SLEEP_TIME);
        int random_player_index;
        do
            random_player_index = rand() % num + offset;
        while (strcmp(player_pointers[random_player_index]->state,"OUT_OF_THE_GAME")==0);
        pthread_t *pRandom_player = player_pointers[random_player_index]->thread;
        pthread_kill(*pRandom_player, signum);
        
    }
}

void *seeker_function(void *p)
{
    for (;;)
    {
        random_sleep(MAX_SEEKER_SLEEP_TIME);
        printf("Reaching for Golden Snitch...");
        if (Caught_Snitch == 1)
        {
            for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
            {
                if (strcmp(player_pointers[i]->state,"OUT_OF_THE_GAME")==0)
                    continue;
                if (*(player_pointers[i]->thread) == pthread_self())
                {
                    if (player_pointers[i]->team == 'A')
                           team_a_score += 150;
                    else if (player_pointers[i]->team == 'B')
                         team_b_score += 150;
                    printf("%s CAUGHT THE SNITCH!!! GAME OVER!!!\n\n", player_pointers[i]->name);
                    printf("Final Scores - Team A: %i, Team B: %i\n\n", team_a_score, team_b_score);
                    if(team_a_score>team_b_score)
                       printf("Team A won the game...\n");
                    else
                       printf("Team B won the game...\n");
                    exit(0);
                }
            }
        }
        else{
            for (int i = 0; i < NUMBER_OF_PLAYERS; ++i){
              if (*(player_pointers[i]->thread) == pthread_self())
                   printf("%s missed the Golden Snitch...\n",player_pointers[i]->name);
      }
    }
  }
}

void *keeper_function(void *p)
{
    for (;;)
    {
        random_sleep(MAX_KEEPER_SLEEP_TIME);
        for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
        {
            if (strcmp(player_pointers[i]->state,"OUT_OF_THE_GAME")==0)
                continue;
            if (*(player_pointers[i]->thread) == pthread_self())
            {
                pthread_t goal = player_pointers[i]->team == 'A' ? goal_a : goal_b;
                pthread_kill(goal, SIGUSR1);
            }
        }
    }
}

void *snitch_function(void *p)
{
    for (;;)
    {
        random_sleep(MAX_SNITCH_SLEEP_TIME);
        Caught_Snitch = 1;
        sleep(1);
        Caught_Snitch = 0;
    }
}

void *chaser_function(void *p)
{
    // Chaser waits to receive the quaffle, then makes a goal attempt. This is done via an sa_handler
    for (;;)
        ;
}

void *goal_function(void *p)
{
    // Goal waits to receive an attempt on it from a chaser, then adjusts the score if completed. This is done via an sa_handler
    for (;;)
        ;
}

// Signal handlers
void hit_by_bludger_or_goal_attempt(int signum)
{
    // Determine if this SIGINT was sent to a player (representing a bludger) or a goal (representing a quaffle)
    int is_player = 0;
    for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
    {
        if (*(player_pointers[i]->thread) == pthread_self())
        {
            is_player = 1;
            break;
        }
    }

    if (is_player == 0)
    {
       
        if (pthread_self() == goal_a){
            printf("Attempt on goal A!!!\n");
            Goal_A=1;
            sleep(2);
            if(Goal_A==0){
                  printf("Team A KEEPER saved the Goal!!!!\n");                 
               }
            if(Goal_A==1){
                  team_b_score += 10;
                  printf("A goal was scored by CHASER of Team B...\n");
                  Goal_A=0;                 
               }
           }
        else{
            printf("Attempt on goal B!!!\n");
            Goal_B=1;
            sleep(2);
            if(Goal_B==0){
                  printf("Team B KEEPER saved the Goal!!!!!\n");                 
               }
            if(Goal_B==1){
                  team_a_score += 10;
                  printf("A goal was scored by CHASER of Team A...\n");
                  Goal_B=0;;                 
               }
           }
            
        printf("Scores - Team A: %i, Team B: %i\n\n", team_a_score, team_b_score);
    }
    else
    {
        char print_string[35];
        int i=0;
        for (i=0; i < NUMBER_OF_PLAYERS; ++i)
        {
            if (strcmp(player_pointers[i]->state,"OUT_OF_THE_GAME")==0)
                continue;
            if (*(player_pointers[i]->thread) == pthread_self())
            {   
                printf("%s is attacked by Bludger....\n",player_pointers[i]->name);
                strcpy(print_string, player_pointers[i]->name);
                strcpy(player_pointers[i]->state,"ABOUT_TO_BE_HIT_BY_THE_BLUDGER");
                sleep(2);
                if(strcmp(player_pointers[i]->state,"Playing")==0){
                   printf("%s was saved by Beater...\n",player_pointers[i]->name);
                   }
                if(strcmp(player_pointers[i]->state,"ABOUT_TO_BE_HIT_BY_THE_BLUDGER")==0){
                   strcpy(player_pointers[i]->state,"OUT_OF_THE_GAME");
                   printf("%s is OUT_OF_THE_GAME...\n",player_pointers[i]->name);
                   }
                break;
            }
            
        } 
        int count=0;
        for (int j = 0; j < NUMBER_OF_PLAYERS; ++j){
           if(strcmp(player_pointers[j]->state,"Playing")==0 || strcmp(player_pointers[j]->state,"ABOUT_TO_BE_HIT_BY_THE_BLUDGER")==0)
                  count+=1;
        }
        if(strcmp(player_pointers[i]->state,"OUT_OF_THE_GAME")==0){
                printf("Total number of Players left: %d after %s was HIT by Bludger...\n",count,print_string);
                pthread_exit(0);
                }
        else if(strcmp(player_pointers[i]->state,"Playing")==0){
                printf("Total number of Players left: %d after %s was SAVED by Beater...\n",count,print_string);
            }
        
    }
}

void saved_by_beater_or_goal_blocked(int signum)
{
    // Determine if this SIGUSR1 was sent to a player (representing a bludger) or a goal (representing a quaffle)
    int is_player = 0;
    for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
    {
        if (*(player_pointers[i]->thread) == pthread_self())
        {
            is_player = 1;
            if(strcmp(player_pointers[i]->state,"ABOUT_TO_BE_HIT_BY_THE_BLUDGER")==0)
                strcpy(player_pointers[i]->state,"Playing");    
        }
    }
    
    
    if (is_player == 0){
            if (pthread_self() == goal_a){
                 //player_pointers[2] points to keeper_a_struct..
                 if(strcmp(player_pointers[2]->state,"ABOUT_TO_BE_HIT_BY_THE_BLUDGER")==0 || strcmp(player_pointers[2]->state,"Playing")==0)
                   Goal_A=0;
               }
            else if(pthread_self() == goal_b){
                 //player_pointers[3] points to keeper_b_struct..
                 if(strcmp(player_pointers[3]->state,"ABOUT_TO_BE_HIT_BY_THE_BLUDGER")==0 || strcmp(player_pointers[3]->state,"Playing")==0)
                   Goal_B=0;
               }           
        }
}

void caught_quaffle(int signum)
{
    for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
    {
        if (strcmp(player_pointers[i]->state,"OUT_OF_THE_GAME")==0)
            continue;
        if (*(player_pointers[i]->thread) == pthread_self())
        {
            pthread_t goal = player_pointers[i]->team == 'A' ? goal_b : goal_a;
            printf("%s in possesion of the quaffle...\n",player_pointers[i]->name);
            pthread_kill(goal, SIGINT);
        }
    }
}

void random_sleep(int time)
{
    int sleep_time = rand() % time;
    sleep_time += MIN_SLEEP_TIME;
    sleep((unsigned int)sleep_time);
}