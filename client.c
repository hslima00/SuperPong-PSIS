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
    mvwprintw(controls_and_info, 1, 1 ,"IF ON PLAY STATE: ");
    mvwprintw(controls_and_info, 2 , 1 ,"Use the arrows or wasd to control the paddle\0");
    mvwprintw(controls_and_info, 3 , 1 ,"Press \tq to leave the game  ");
    mvwprintw(controls_and_info, 4 , 1 ,"Press \tr to release the ball after at least 10 secs ");
    wrefresh(controls_and_info);
}

/*initial paddle position*/
void new_paddle (paddle_position * paddle, int length){ 
    paddle->x = WINDOW_SIZE/2;
    paddle->y = WINDOW_SIZE-2;
    paddle->length = length;
}

void draw_paddle(WINDOW *win, paddle_position * paddle, int draw){
    int ch;
    if(draw){
        ch = 61 ;
    }else{
        ch = ' ';
    }
    int start_x = paddle->x - paddle->length;
    int end_x = paddle->x + paddle->length;
    for (int x = start_x; x <= end_x; x++){
        wmove(win, paddle->y, x);
        waddch(win,ch);
    }
    wrefresh(win);
}

void move_paddle (paddle_position * paddle, int direction, WINDOW *message_win ){
    mvwprintw(message_win, 3,1,"                        ");
    wrefresh(message_win);
    if (direction == KEY_UP || direction == 'w'){
        if (paddle->y  != 1){
            paddle->y --;
            mvwprintw(message_win, 3,1,"UP movement\0");
        }
        else mvwprintw(message_win, 3,1,"Hitting BOX limit\0"); 
    }
    if (direction == KEY_DOWN || direction == 's'){
        if (paddle->y  != WINDOW_SIZE-2){
            paddle->y ++;
            mvwprintw(message_win, 3,1,"DOWN movement\0");
        }
        else mvwprintw(message_win, 3,1,"Hitting BOX limit\0"); 
    }
    if (direction == KEY_LEFT || direction == 'a'){
        if (paddle->x - paddle->length != 1){
            paddle->x --;
            mvwprintw(message_win, 3,1,"LEFT movement\0");
        }
        else mvwprintw(message_win, 3,1,"Hitting BOX limit\0"); 
    }
    if (direction == KEY_RIGHT || direction == 'd'){
        if (paddle->x + paddle->length != WINDOW_SIZE-2){
            paddle->x ++;
            mvwprintw(message_win, 3,1,"RIGHT movement\0");
        }
        else mvwprintw(message_win, 3,1,"Hitting BOX limit\0"); 
    }
    wrefresh(message_win);
}

void move_ball(ball_position_t * ball){
    
    int next_x = ball->x + ball->left_ver_right;
    if( next_x == 0 || next_x == WINDOW_SIZE-1){
        ball->up_hor_down = rand() % 3 -1 ;
        ball->left_ver_right *= -1;
        //mvwprintw(message_win, 2,1,"left right win");
        //wrefresh(message_win);
     }else{
        ball->x = next_x;
    }

    
    int next_y = ball->y + ball->up_hor_down;
    if( next_y == 0 || next_y == WINDOW_SIZE-1){
        ball->up_hor_down *= -1;
        ball->left_ver_right = rand() % 3 -1;
       // mvwprintw(message_win, 2,1,"bottom top win");
        //wrefresh(message_win);
    }else{
        ball -> y = next_y;
    }
}

void draw_ball(WINDOW *win, ball_position_t * ball, int draw){
    int ch;
    if(draw){
        ch = ball->c;
    }else{
        ch = ' ';
    }
    wmove(win, ball->y, ball->x);
    waddch(win,ch);
    //wrefresh(win);
}

void simulate_ball_position( paddle_position * paddle, message * m, WINDOW * message_win, int key ,int *scores){
    ball_position_t simul_ball;
    simul_ball.c = m->ball_position.c;
    simul_ball.left_ver_right = m->ball_position.left_ver_right;
    simul_ball.up_hor_down = m->ball_position.up_hor_down;
    simul_ball.x = m->ball_position.x;
    simul_ball.y = m->ball_position.y;
    move_ball(&simul_ball);
    int condition_1 = ((simul_ball.x <= paddle->x + paddle->length) && (simul_ball.x >= paddle->x - paddle->length)) && (simul_ball.y == paddle->y);
    if (condition_1) {
        scores[0]++;
        m->point = TRUE;
        m->ball_position.up_hor_down *= -1;
        if (m->ball_position.up_hor_down == 0){
            if(key=='s' || key==KEY_DOWN)m->ball_position.up_hor_down = 1;
            else if(key=='w' || key==KEY_UP)m->ball_position.up_hor_down = -1;
            else  m->ball_position.left_ver_right *=-1;
        }
    
    }
    else if (simul_ball.y == paddle->y && !(m->ball_position.up_hor_down==0))scores[1]++;
    mvwprintw(message_win, 2,1,"SCORED - %d \t|\tMISSED - %d", scores[0], scores[1]);
    wrefresh(message_win);
}

void update_scoreboard(int score[], WINDOW * score_win){
    int temp_score[MAX_CLIENTS], temp_max = 0, k = 0 , clt_on_score = -1;
    for (int j = 0; j < MAX_CLIENTS; j++){
        temp_score[j] = -1;
        mvwprintw(score_win, j+2 , 1 ,"                      \0"); 
        wrefresh(score_win); 
        if(score[clt_on_score +1 ]!= -1){
            ++ clt_on_score;
            temp_score[clt_on_score] = score[clt_on_score]; 
        } 
    }
    for (int l = 0; l <= clt_on_score; l++){
        for (int j = clt_on_score ;j >= 0; j--){
            if (temp_score[j] >= temp_max ) {
                temp_max = temp_score[j];
                k = j;
            }
        } 
        mvwprintw(score_win, l+2 , 1 ," %dº | PLAYER %d |%d\0 ", l+1 , k ,temp_score[k]);
        wrefresh(score_win);
        temp_score[k] = -1;
        temp_max = 0;
    } 
    
}

void update_ball_and_paddle(WINDOW *my_win, message * m, paddle_position * paddle, int key,WINDOW *message_win, int * scores){
    draw_paddle(my_win, paddle, FALSE);                 //deletes paddle
    move_paddle (paddle, key, message_win);             //calculates new paddle position according to user input
    draw_ball(my_win, &m->ball_position, FALSE);  //deletes ball  
    simulate_ball_position( paddle, m , message_win, key, scores); //simulates if ball will hit paddle or not,
                                                                         //if it does, changes direction of ball
    draw_paddle(my_win, paddle, TRUE);              
    move_ball(&m->ball_position);                       
    draw_ball(my_win, &m->ball_position, TRUE);
    wrefresh(my_win);
}//delete previous ball, calculate new position and draws it 

void save_ball_position(ball_position_t * previous_ball, ball_position_t * position_to_save){
    previous_ball->c = position_to_save->c;
    previous_ball->x = position_to_save->x;
    previous_ball->y = position_to_save->y;
    previous_ball->left_ver_right = position_to_save->left_ver_right;
    previous_ball->up_hor_down = position_to_save->up_hor_down; 
}//

void clear_paddle_nd_msg_window(WINDOW * my_win, paddle_position * paddle, WINDOW * message_win){
    draw_paddle(my_win, paddle, FALSE);
    mvwprintw(message_win, 1,1,"                                        \0");
    mvwprintw(message_win, 2,1,"                                        \0");
    mvwprintw(message_win, 3,1,"                                        \0");
    mvwprintw(message_win, 4,1,"                                        \0");
}

void start_play_state(WINDOW * my_win, paddle_position * paddle, message * m ,WINDOW * message_win){
    mvwprintw(message_win, 1,1,"PLAY STATE");
    draw_paddle(my_win , paddle, TRUE);// draws paddle in the defined position (mode TRUE=draw/ mode FALSE= delete)
    draw_ball(my_win,&m->ball_position,TRUE);
    wrefresh(my_win);
    wrefresh(message_win);
    m->point = FALSE;
    m->msg_type = 3;
}

void connect_message(WINDOW * message_win){
    mvwprintw(message_win, 1,1,"SUCESS IN CONNECTION WAIT \0");
    wrefresh(message_win);
     mvwprintw(message_win, 1,1,"                                        \0");
}

void movement_message(WINDOW * my_win , message * m){
    wrefresh(my_win);
    draw_ball(my_win, &m->ball_position, TRUE);
    wrefresh(my_win);
    draw_ball(my_win, &m->ball_position, FALSE);
}
int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("to run client supply the server adress after ./CLIENT.proj \n");
        exit(0);
    }
    char* adress_keyboard=argv[1];
    int sock_fd , condition, key = -1,scores [2]; 
    scores[0] = 0;
    scores[1] = 0;        
    message m;
    paddle_position paddle;
    struct sockaddr_in server_addr;

    criar_socket(&sock_fd);
    
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
    WINDOW * controls_and_info= newwin(6, WINDOW_SIZE + 25, WINDOW_SIZE+5, 0);// message window with infomation if in PLAY_STATE
    new_paddle(&paddle , PADDLE_SIZE);// initializes all paddle arguments 
     //TODO_9
    // prepare the movement message  
    
    do{
        recv(sock_fd, &m, sizeof(message), 0);
        if(m.msg_type==5){
            mvwprintw(max_player_win, 1,1,"MAX PLAYER NUMBER EXCEEDED, PROCESS WILL BE KILLED IN 5s");
            wrefresh(max_player_win);
            sleep(5);
            break;
        }
        create_windows(my_win,message_win, score_win, controls_and_info);
        if(m.msg_type==0){
            connect_message(message_win);
            continue;
        }
        
        if(m.point) update_scoreboard(m.cinfo[m.client_contacting].score, score_win);
        switch(m.msg_type){
            case 2:// send _ball == playstate
                do{   
                    start_play_state(my_win, &paddle, &m, message_win);
                    key = wgetch(my_win);
                    condition = (key == KEY_LEFT ||  KEY_RIGHT || KEY_UP || KEY_DOWN ||  'a'|| 's' ||'d' ||  'w');
                    if(key == 'q'){
                        m.msg_type = 4; // disconnect message
                        sendto(sock_fd, &m, sizeof(message), 0,
                            (const struct sockaddr *)&server_addr, sizeof(server_addr));
                        endwin();// End curses mode	
                        return 0;
                    }
                    else if(condition) update_ball_and_paddle(my_win, &m,&paddle, key, message_win, scores);
                }while((!(condition)) && (m.msg_type!=1));            
                
                sendto(sock_fd, &m, sizeof(message), 0,
                        (const struct sockaddr *)&server_addr, sizeof(server_addr)); //send the move ball message
                wrefresh(message_win);
                break;
            case 3://move_ball
                movement_message(my_win, &m);
                break;
               
            default: 
                wrefresh(message_win);
        }
    }while(key !=27);
   
    
    endwin();// End curses mode	
    return 0;    
}