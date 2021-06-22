#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>

//Matrix constants
#define ROWS 15
#define COLS 30
#define EMPTY_CELL '/'
#define FOOD '*'

//Movements
#define UP 'w'
#define DOWN 's'
#define LEFT 'a'
#define RIGHT 'd'

//Snake constants
#define SNAKE '@'
#define DEFAULT_DIM 1

//Game constants
#define MAX_SCORE 26
#define QUIT 'q'

#define clear() system("clear")

struct termios old;

//The getch() function get a character without display it on terminal
int getch(){
    char c;
    struct termios current;

    tcgetattr(0, &old);

    current = old;
    current.c_lflag &= ~ICANON; // disable buffered input
    current.c_lflag &= ~ECHO; // disable echo
    
    tcsetattr(0, TCSANOW, &current); // apply new settings
    
    c = getchar();
    if(c == '\n') c = '\r';

    tcsetattr(0, TCSANOW, &old); // restore old settings

    return c;
}

typedef enum{
    false,
    true
}boolean;

typedef struct{
    short abs[MAX_SCORE + 1], ord[MAX_SCORE + 1];
    short size;
}snake;

snake s = {
    .abs[0] = 0,
    .ord[0] = 0,
    .size = DEFAULT_DIM
};
char matr[ROWS][COLS], c = RIGHT;
pthread_t tid;
boolean running = true;

void swap(char *a, char *b){
    *a = *b + *a;
    *b = *a - *b;
    *a = *a - *b;
}

void stop(){
    running = false;
    tcsetattr(0, TCSANOW, &old);
}

int check_food(){
    boolean value = false;
    int i, j;

    for(i = 0; i < ROWS; i++)
        for(j = 0; j < COLS; j++)
            if(matr[i][j] == FOOD) value = true;
    
    return value;
}

void generate_point(){
    boolean value;
    short x, y;
    
    value = true;

    do{
        x = clock() % COLS;
        y = clock() % ROWS;
         
        if(matr[y][x] != '@'){
            matr[y][x] = FOOD;
            value = false;
        }
    }while(value);
}

void elaborate_map(){
    int i, j;
    
    for(i = 0; i < ROWS; i++){
        for(j = 0; j < COLS; j++){
            if(matr[i][j] != FOOD)
                matr[i][j] = EMPTY_CELL;
        }
    }
    
    for(i = 0; i <= s.size; i++)
        matr[s.ord[i]][s.abs[i]] = SNAKE;
}

void render_map(){
    clear();
    
    if(s.size == MAX_SCORE){
        printf("You have won.\n");
        clear();
        printf("Game Over\n");
    }
    
    printf("Your score: %d\n", (s.size - 1));

    for(int i = 0; i < ROWS; i++){
        for(int j = 0; j < COLS; j++){
            putchar(matr[i][j]);
        }
        putchar('\n');
    }
}

void change_coordinates(){
    for(int i = s.size; i > 0; i--){
        s.abs[i] = s.abs[i - 1];
        s.ord[i] = s.ord[i - 1];
    }
}

void lose(){
    clear();
    printf("Game Over\n");
    stop();
}

void move_snake(char dir){
    switch(dir){
        case UP:
            if(s.ord[0] != 0){
                if(matr[s.ord[0] - 1][s.abs[0]] == '@') lose();
                s.ord[0]--;
                change_coordinates();
            } else{
                lose();
            }
            break;
        
        case DOWN:
            if(s.ord[0] != (ROWS - 1)){
                if(matr[s.ord[0] + 1][s.abs[0]] == '@') lose();
                s.ord[0]++;
                change_coordinates();
            } else{
                lose();
            }
            break;
        
        case LEFT:
            if(s.abs[0] != 0){
                if(matr[s.ord[0]][s.abs[0] - 1] == '@') lose();
                s.abs[0]--;
                change_coordinates();
            } else{
                lose();
            }
            break;
        
        case RIGHT:
            if(s.abs[0] != (COLS - 1)){
                if(matr[s.ord[0]][s.abs[0] + 1] == '@') lose();
                s.abs[0]++;
                change_coordinates();
            } else{
                lose();
            }
            break;
        
        case QUIT:
            stop();
            break;
    }

    if(matr[s.ord[0]][s.abs[0]] == FOOD){
        s.size++;
        s.abs[s.size] = s.abs[s.size - 1];
        s.ord[s.size] = s.ord[s.size - 1];
        generate_point();
    }
    
    elaborate_map();
}

void *wait_char(void *arg){
    while(running){
        c = tolower(getch());
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv){
    
    generate_point();
    elaborate_map();

    pthread_create(&tid, NULL, wait_char, NULL);

    do{
        render_map();
        usleep(500000);
        move_snake(c);
    }while(running);
    
    return 0;
}
