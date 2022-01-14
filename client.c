#include "header.h"

void criar_socket(int *sock_fd){
    *sock_fd = socket(AF_INET, SOCK_DGRAM, 0);   //cria socket
    if (*sock_fd == -1){                         //verifica se socket esta bem criada
	    perror("socket: ");   
	    exit(-1);
    }

}

void initialize_ncurses(){
    initscr();		    	
	cbreak();				
    keypad(stdscr, TRUE);   
	noecho();
    curs_set(0);
}
/* creates the 3 windows and draws a border for each */
void create_windows(WINDOW * my_win , WINDOW * message_win , WINDOW * score_win, WINDOW * controls_and_info){   
    box(my_win, 0 , 0);	
	wrefresh(my_win);
    keypad(my_win, true);
    box(message_win, 0 , 0);	
	wrefresh(message_win);
    box(score_win, 0 , 0);	
    mvwprintw(score_win, 1 , 2 ,"SCOREBOARD");
    wrefresh(score_win);
    box(controls_and_info, 0 , 0);	
    mvwprintw(controls_and_info, 2 , 1 ,"Use the arrows or wasd to control the paddle\0");
    mvwprintw(controls_and_info, 3 , 1 ,"Press \tq to leave the game  ");
    wrefresh(controls_and_info);
}

void draw_paddle(WINDOW *my_win, paddle_position * paddle, bool draw, bool this_client){
    int ch;
    if(draw){
        if (this_client)ch = '=' ;
        else ch = '_';
    }else{
        ch = ' ';
    }
    int start_x = paddle->x - paddle->length;
    int end_x = paddle->x + paddle->length;
    for (int x = start_x; x <= end_x; x++){
        wmove(my_win, paddle->y, x);
        waddch(my_win,ch);
    }
    wrefresh(my_win);
}

void move_paddle (message * m, int direction, WINDOW *message_win ){
    mvwprintw(message_win, 3,1,"                        ");
    wrefresh(message_win);
    if (direction == KEY_UP || direction == 'w'){
        if (m->cinfo[m->client_contacting].paddle_position.y  != 1){
            m->cinfo[m->client_contacting].paddle_position.y --;
            mvwprintw(message_win, 3,1,"UP movement\0");
        }
        else mvwprintw(message_win, 3,1,"Hitting BOX limit\0"); 
    }
    if (direction == KEY_DOWN || direction == 's'){
        if (m->cinfo[m->client_contacting].paddle_position.y != WINDOW_SIZE-2){
            m->cinfo[m->client_contacting].paddle_position.y ++;
            mvwprintw(message_win, 3,1,"DOWN movement\0");
        }
        else mvwprintw(message_win, 3,1,"Hitting BOX limit\0"); 
    }
    if (direction == KEY_LEFT || direction == 'a'){
        if (m->cinfo[m->client_contacting].paddle_position.x - m->cinfo[m->client_contacting].paddle_position.length  != 1){
            m->cinfo[m->client_contacting].paddle_position.x--;
            mvwprintw(message_win, 3,1,"LEFT movement\0");
        }
        else mvwprintw(message_win, 3,1,"Hitting BOX limit\0"); 
    }
    if (direction == KEY_RIGHT || direction == 'd'){
        if (m->cinfo[m->client_contacting].paddle_position.x + m->cinfo[m->client_contacting].paddle_position.length != WINDOW_SIZE-2){
            m->cinfo[m->client_contacting].paddle_position.x ++;
            mvwprintw(message_win, 3,1,"RIGHT movement\0");
        }
        else mvwprintw(message_win, 3,1,"Hitting BOX limit\0"); 
    }
    m->point = (( ( (m->ball_position.x <=m->cinfo[m->client_contacting].paddle_position.x + PADDLE_SIZE) &&       //extremidade direita do paddle
            m->cinfo[m->client_contacting].paddle_position.x - PADDLE_SIZE) ) &&          //extremidade esquerda do paddle 
            (m->cinfo[m->client_contacting].paddle_position.y == m->ball_position.y));                           //estiver entre as extremidades do paddle e no mesmo y 
    wrefresh(message_win);
}

void draw_ball(WINDOW *my_win, ball_position_t * ball, bool draw){
    int ch;
    if(draw){
        ch = 'o';
    }else{
        ch = ' ';
    }
    wmove(my_win, ball->y, ball->x);
    waddch(my_win,ch);
    wrefresh(my_win);
}

void update_scoreboard(client_info_t * cinfo, WINDOW * score_win, int my_ID){
    int temp_score[MAX_CLIENTS][2], temp_max = 0, k = 0 , clt_on_score = -1;
    
    for (int j = 0; j < MAX_CLIENTS; j++){
        temp_score[j][1] = -1;
        mvwprintw(score_win, j+2 , 1 ,"                        \0"); 
        if(cinfo[clt_on_score +1 ].score!= -1){
            ++ clt_on_score;
            temp_score[clt_on_score][1] = cinfo[clt_on_score].score; 
            temp_score[clt_on_score][2] = cinfo[clt_on_score].client_ID;
        } 
    }
    for (int l = 0; l <= clt_on_score; l++){
        for (int j = clt_on_score ;j >= 0; j--){
            if (temp_score[j][1] >= temp_max ) {
                temp_max = temp_score[j][1];
                k = j;
            }
        } 
        if(temp_score[k][2] == my_ID)mvwprintw(score_win, l+2 , 1 ,"-->%dº | PLAYER %d |%d\0 ", l+1 , temp_score[k][2] ,temp_score[k][1]);
        else mvwprintw(score_win, l+2 , 3 ," %dº | PLAYER %d |%d\0 ", l+1 , temp_score[k][2] ,temp_score[k][1]);
        wrefresh(score_win);
        temp_score[k][1] = -1;
        temp_max = 0;
    } 
    
}

void update_paddle(WINDOW *my_win, message * m, paddle_position * paddle, int key,WINDOW *message_win){
    draw_paddle(my_win,paddle,FALSE,TRUE);
    move_paddle (m, key, message_win);
    paddle->x =m->cinfo[m->client_contacting].paddle_position.x;
    paddle->y =m->cinfo[m->client_contacting].paddle_position.y; 
    draw_paddle(my_win,paddle,TRUE,TRUE);
     
}

void update__all_paddles_and_ball(WINDOW *my_win, message * m, paddle_position * paddles, bool start,ball_position_t *previous_ball){
    for (int j = 0; j < MAX_CLIENTS; j++){
        if(m->cinfo[j].score!= -1){
            if(!start){
                draw_paddle(my_win , &paddles[j], FALSE,FALSE);
            }
            paddles[j].length = PADDLE_SIZE;
            paddles[j].x= m->cinfo[j].paddle_position.x;
            paddles[j].y= m->cinfo[j].paddle_position.y;
            if (j == m->client_contacting)draw_paddle(my_win , &m->cinfo[j].paddle_position, TRUE, TRUE);
            else draw_paddle(my_win , &m->cinfo[j].paddle_position, TRUE, FALSE);
        }
        else break;
    }
    if(!start)draw_ball(my_win, previous_ball,FALSE);
    previous_ball->x = m->ball_position.c;
    previous_ball->x =m->ball_position.x;
    previous_ball->y =m->ball_position.y;
    previous_ball->left_ver_right = m->ball_position.left_ver_right;
    previous_ball->up_hor_down= m->ball_position.left_ver_right;
    draw_ball(my_win, previous_ball,TRUE);
}

void start_play_state(WINDOW * my_win,  message * m ,WINDOW * message_win, paddle_position *paddles, ball_position_t * previous_ball){
    mvwprintw(message_win, 1,1,"PLAY STATE");
    update__all_paddles_and_ball(my_win,m,paddles,TRUE,previous_ball);
    wrefresh(message_win);
}

void update_board(message *m, WINDOW * score_win, WINDOW * my_win,paddle_position *paddles,ball_position_t *previous_ball ,int my_ID){
    update_scoreboard(m->cinfo, score_win, my_ID);
    update__all_paddles_and_ball(my_win,m,paddles,FALSE,previous_ball);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("to run client supply the server adress after ./CLIENT.proj \n");
        exit(0);
    }
    char* adress_keyboard=argv[1];
    int sock_fd , condition, key = -1;      
    message m;
    paddle_position paddles[MAX_CLIENTS];
    ball_position_t previous_ball;
    struct sockaddr_in server_addr;
    bool sucess_connect = TRUE;
    criar_socket(&sock_fd);
    
    int my_ID;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SOCK_PORT);
    if( inet_pton(AF_INET, adress_keyboard, &server_addr.sin_addr) < 1){
        //inet_pton - convert IPv4 and IPv6 addresses from text to binary form
		printf("no valid address: \n");
		exit(-1);
	}
    //STEP A
    // send connection message
    
    m.msg_type = 0;  //connect;
    sendto(sock_fd, &m, sizeof(message), 0,
          (const struct sockaddr *)&server_addr, sizeof(server_addr)); //send the connection message
    
    initialize_ncurses();
    WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);// main window for game
    WINDOW * max_player_win = newwin(4, WINDOW_SIZE, 0, 0);//window displayed when max players are exceed 
    WINDOW * message_win = newwin(5, WINDOW_SIZE + 25, WINDOW_SIZE, 0);// message window with infomation if in PLAY_STATE
    WINDOW * score_win = newwin(WINDOW_SIZE , 24 , 0 , WINDOW_SIZE+1);//Score window at the right side of the game with player scores
    WINDOW * controls_and_info= newwin(5, WINDOW_SIZE + 25, WINDOW_SIZE+5, 0);// message window with infomation if in PLAY_STATE
     //TODO_9
    // prepare the movement message  
    
    do{
        recv(sock_fd, &m, sizeof(message), 0);
        if(m.msg_type==4){
            mvwprintw(max_player_win, 1,1,"MAX PLAYER NUMBER EXCEEDED, PROCESS WILL BE KILLED IN 5s");
            wrefresh(max_player_win);
            sleep(5);
            break;
        }
        else if (sucess_connect){ 
            create_windows(my_win,message_win, score_win, controls_and_info);
            start_play_state(my_win, &m, message_win, paddles, &previous_ball);
            my_ID = m.cinfo[m.client_contacting].client_ID;
            update_scoreboard(m.cinfo, score_win, my_ID);
            sucess_connect = FALSE;
            }
        else update_board(&m, score_win, my_win,paddles,&previous_ball,my_ID);
        m.msg_type = 3;
        do{  
            key = wgetch(my_win);
            condition = (key == KEY_LEFT ||  KEY_RIGHT || KEY_UP || KEY_DOWN ||  'a'|| 's' ||'d' ||  'w');
            if(key == 'q'){
                m.msg_type = 1; // disconnect message
                sendto(sock_fd, &m, sizeof(message), 0,
                    (const struct sockaddr *)&server_addr, sizeof(server_addr));
                endwin();// End curses mode	
                return 0;
            }
            else if(condition) update_paddle(my_win, &m,&paddles[m.client_contacting], key, message_win);
        }while(!condition);            
        m.msg_type = 2;
        sendto(sock_fd, &m, sizeof(message), 0,
                (const struct sockaddr *)&server_addr, sizeof(server_addr)); //send the move ball message
        wrefresh(message_win);
    }while(key !=27);
   
    
    endwin();// End curses mode	
    return 0;    
}