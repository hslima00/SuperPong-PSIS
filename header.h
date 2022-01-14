#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <ncurses.h>

#define SOCK_PORT 4000
#define MAX_CLIENTS 3 // mudar para 10 later on

#define WINDOW_SIZE 30
#define PADDLE_SIZE 3

typedef enum direction_t {UP, DOWN, LEFT, RIGHT} direction_t;

typedef struct ball_position_t{
    int x, y;
    int up_hor_down; //  -1 up, 0 horizontal, 1 down
    int left_ver_right; //  -1 left, 0 vertical,1 right
    char c;
} ball_position_t;

typedef struct paddle_position{
    int x, y;
    int length;
} paddle_position;

typedef struct client_info_s
{
    //Aquilo que o server vai guardar 
    char client_address_s[INET_ADDRSTRLEN];
    int port;
    int score;
    int client_ID; 
    struct paddle_position paddle_position_s; //paddle position server
}client_info_s;

//[0<i<clients_online]
// m.client_contacting = i;
//
typedef struct client_info_t{
    int score;
    paddle_position paddle_position;
    int client_ID;  // index that can change always < 10 and basicly is the position of the matrix score to look at
}client_info_t;

typedef struct message 
{  
    int client_contacting;
    int msg_type;   /* 0-connect   1-disconnect  2-Paddle_move 3-Board_update  4-MAX PLAYERS EXCEED*/  
    client_info_t cinfo[MAX_CLIENTS]; 
    ball_position_t ball_position;
    //int score[MAX_CLIENTS][2]; //matrix 10 with score value and m.cinfo.cliend_ID [1] score [2 client number]
    bool point; 
}message;