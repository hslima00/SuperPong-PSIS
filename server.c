#include "header.h"
//Creates and binds socket
void create_socket(int *sock_fd)
{
    *sock_fd = socket(AF_INET, SOCK_DGRAM, 0); //cria socket
    if (*sock_fd == -1)
    { //verifica se socket esta bem criada
        perror("socket: ");
        exit(-1);
    }
}
//Self-explanatory
void initialize_ncurses()
{
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
}
//Creates a window and draws a border
void create_window(WINDOW *my_win)
{
    
    box(my_win, 0, 0);
    wrefresh(my_win);
    keypad(my_win, true);
}

void copy_ball_info(ball_position_t* ball_to_overwrite, ball_position_t *ball_to_copy_from){
    ball_to_overwrite->c= ball_to_copy_from->c;
    ball_to_overwrite->left_ver_right= ball_to_copy_from->left_ver_right;
    ball_to_overwrite->up_hor_down= ball_to_copy_from->up_hor_down;
    ball_to_overwrite->x= ball_to_copy_from->x;
    ball_to_overwrite->y= ball_to_copy_from->y;
}
//At the start of the game places the ball in a random position
void place_ball_random(ball_position_t *ball,ball_position_t *ball_s )
{
    ball->x = rand() % WINDOW_SIZE;
    ball->y = rand() % WINDOW_SIZE;
    ball->c ='o';
    ball->up_hor_down = rand() % 3 - 1;    //  -1 up, 1 - down
    ball->left_ver_right = rand() % 3 - 1; // 0 vertical, -1 left, 1 right
    //initialize ball position on server
    copy_ball_info(ball_s, ball);
}

bool new_paddle (message *m, int adder, int clients_online){ 
    paddle_position simul_paddle; 
    int i = 0;
    simul_paddle.length = PADDLE_SIZE;
    simul_paddle.x = WINDOW_SIZE/2;
    simul_paddle.y = WINDOW_SIZE-2 - (clients_online + adder);
    /*if ((WINDOW_SIZE-2 + clients_online)%2){ 
        simul_paddle.y = WINDOW_SIZE-2 - clients_online - adder;

    }else{
        simul_paddle.y = -2 + clients_online +adder; 
    }*/
    do{
        if(simul_paddle.y == m->cinfo[i].paddle_position.y)return TRUE; // se encontrar um paddle dá return 1  
        i++;
    }while (i != clients_online);
    
    //se não encontrar paddle já criada, entao vai passar as cenas do paddle simulado para o cliente 
    m->cinfo[clients_online-1].paddle_position.length = PADDLE_SIZE;
    m->cinfo[clients_online-1].paddle_position.x=simul_paddle.x;
    m->cinfo[clients_online-1].paddle_position.y=simul_paddle.y;
    return FALSE;
}



void move_ball(message *m, client_info_s *cinfo_s, int clients_online, ball_position_t * ball_s){
    ball_position_t next_ball; 
    // simullates if ball will hit de window
    int limite_esq = 1; 
    int limite_dir = WINDOW_SIZE-2;
    int limite_topo = 1;
    int limite_fundo = WINDOW_SIZE-2;
    int next_y, next_x;
    next_ball.left_ver_right =ball_s->left_ver_right;
    next_ball.up_hor_down= ball_s->up_hor_down;
    next_ball.x=  ball_s->x;
    next_ball.y= ball_s->y;

    if(m->point){
        next_x=ball_s->x;
        next_y=ball_s->y;
    }else{
        next_x =ball_s->x + ball_s->left_ver_right;
        next_y =ball_s->y + ball_s->up_hor_down;
    }
   
    if( next_x == 0 || next_x == WINDOW_SIZE-1){
        next_ball.up_hor_down = rand() % 3 -1 ;
        next_ball.left_ver_right *= -1;
     }else{
        next_ball.x = next_x;
    }
    if( next_y == 0 || next_y == WINDOW_SIZE-1){
        next_ball.up_hor_down *= -1;
        next_ball.left_ver_right = rand() % 3 -1;
    }else{
        next_ball.y = next_y;
    }
    
    bool hit;
    //check if ball will colide with any paddle
    for(int j =0; j<clients_online; j++){                                               //se a proxima bola estiver entre:
        hit =( ( (next_ball.x <= cinfo_s[j].paddle_position_s.x + PADDLE_SIZE) &&       //extremidade direita do paddle
            (next_ball.x >= cinfo_s[j].paddle_position_s.x - PADDLE_SIZE) ) &&          //extremidade esquerda do paddle 
            (cinfo_s[j].paddle_position_s.y == next_ball.y));                           //estiver entre as extremidades do paddle e no mesmo y 

        if(hit){
            //atualizar score no server e na msg 
            m->cinfo[j].score++;
            cinfo_s[j].score++;
            m->point=TRUE; // -> força atulizar o score no client 
            if(next_ball.up_hor_down == 1 || next_ball.up_hor_down==-1){    //se a bola estiver a vir para baixo ou para cima
                next_ball.up_hor_down*=-1;  //muda de direção
            }else(next_ball.left_ver_right*=-1);
            
            //se next ball tiver nos limites a direcao q ela ta a andar fa-la ir contra o limite
            if(next_ball.y == limite_fundo) {//fundo
                m -> ball_position.y = WINDOW_SIZE -3;
                m -> ball_position.up_hor_down = -1;
                if(next_ball.x == limite_esq) next_ball.left_ver_right=-1;
                else if(next_ball.x == limite_dir) next_ball.left_ver_right=1; 

            }
            else if(next_ball.y == limite_topo) {//cima 
                m -> ball_position.y = 3;
                m -> ball_position.up_hor_down = 1;
                if(next_ball.x == limite_esq) next_ball.left_ver_right=-1;
                else if(next_ball.x == limite_dir) next_ball.left_ver_right=1;   
            }
            else if(next_ball.x == limite_esq) { //esquerda 
                if (next_ball.left_ver_right!=0)next_ball.left_ver_right= 1;
                if (next_ball.up_hor_down== 0){
                    if(next_ball.y <= WINDOW_SIZE/2)next_ball.up_hor_down=1; //se a bola estiver na metade superior da janela
                    else next_ball.up_hor_down=-1;                           //se "  "  " inferior da janela
                }    
            }           
            else if(next_ball.x == limite_dir) {//dir
                if (next_ball.left_ver_right!=0)next_ball.left_ver_right= -1;
                if (next_ball.up_hor_down== 0){
                    if(next_ball.y <= WINDOW_SIZE/2)next_ball.up_hor_down=1; 
                    else next_ball.up_hor_down=-1;
                }
            }
            
            next_ball.x += 2 * next_ball.left_ver_right;
            next_ball.y += 2 * next_ball.up_hor_down;
        }
    }
    //Save position to ball
    copy_ball_info(&m->ball_position,&next_ball);
    //Save ball position to server
    copy_ball_info(ball_s,&m->ball_position);

}

void paddle_move(paddle_position * client_paddle_after_move,paddle_position * server_saved_position){
    server_saved_position->x = client_paddle_after_move->x;
    server_saved_position->y = client_paddle_after_move->y;
}
//função q chama update ball position -> faz simul ball (todos os clients) -> move ball if valid    

void update_client_info(client_info_s * cinfo_s,message * m, int client_to_update,struct sockaddr_in client_addr,bool remove ){
    //falta address 
    int client_to_get_info_from= client_to_update;
    if (!remove)  {
        struct sockaddr_in* ptr_to_addr = (struct sockaddr_in*)&client_addr;
        struct in_addr addr_to_store = ptr_to_addr->sin_addr;
        inet_ntop( AF_INET, &addr_to_store, cinfo_s[client_to_update].client_address_s, INET_ADDRSTRLEN );
        cinfo_s[client_to_update].port = ntohs(client_addr.sin_port);
    }
    cinfo_s[client_to_update].client_ID = m->cinfo[client_to_get_info_from].client_ID;
    cinfo_s[client_to_update].score = m->cinfo[client_to_get_info_from].score;
    cinfo_s[client_to_update].paddle_position_s.length =PADDLE_SIZE;
    cinfo_s[client_to_update].paddle_position_s.x = m->cinfo[client_to_get_info_from].paddle_position.x;
    cinfo_s[client_to_update].paddle_position_s.y  = m->cinfo[client_to_get_info_from].paddle_position.y;
}

void update_message_info(client_info_s  * cinfo_s, message  * m,int clients_online, ball_position_t ball_s){
    for (int i = 0; i < clients_online; i++)
    {
        m->cinfo[i].client_ID = cinfo_s[i].client_ID;
        m->cinfo[i].paddle_position.length =cinfo_s[i].paddle_position_s.length;
        m->cinfo[i].paddle_position.x =cinfo_s[i].paddle_position_s.x;
        m->cinfo[i].paddle_position.y =cinfo_s[i].paddle_position_s.y;

    }
    for (int i = 0; i < MAX_CLIENTS; i++){
        m->cinfo[i].score = cinfo_s[i].score;
    }
    copy_ball_info(&m->ball_position,&ball_s);
    
}

void new_client_message(message  * m, ball_position_t ball_s, int clients_online){
    m->cinfo[m->client_contacting].score=0;
    m->cinfo[m->client_contacting].paddle_position.length = PADDLE_SIZE;
    m->cinfo[m->client_contacting].paddle_position.x=WINDOW_SIZE/2;
    m->cinfo[m->client_contacting].paddle_position.y=WINDOW_SIZE-2;
    m->msg_type = 3; 
    m->point = TRUE;
    copy_ball_info(&m->ball_position,&ball_s);
}

void add_client(message  * m, struct client_info_s  * cinfo_s, int  clients_online , struct sockaddr_in client_addr,ball_position_t ball_s){
    static int ID=0;
    ID++;
    bool valid; 
    int adder=0; //se encontrar um paddle na posição em que cria esta variavel vai incrementar para aumentar o y 
    m->client_contacting = clients_online -1;  //ISTO DEVE ESTAR MAL PPBLY
    m->cinfo[m->client_contacting].client_ID = ID; //Atribui ID ao cliente a ser criado 
    /*do{
        adder++;
        valid = new_paddle(m, adder,clients_online);
    }while(!valid);*/
    new_client_message(m,ball_s,clients_online);
    update_client_info( cinfo_s, m,  clients_online-1, client_addr,  FALSE);
    update_message_info(cinfo_s, m, clients_online, ball_s);
        
   
}
//Removes clients and wipes its data from server & message
void remove_client(message  * m, struct client_info_s  * cinfo_s ,int  clients_online)
{
    struct sockaddr_in nothing;
    //ciclo que procura o address do gajo que se está a disconectar
    for (int j = m->client_contacting; j < clients_online; j++)
    {
        m->cinfo[j].score= m->cinfo[j+1].score;
        m->cinfo[j].client_ID = m->cinfo[j+1].client_ID;
        strcpy(cinfo_s[j].client_address_s , cinfo_s[j+1].client_address_s);
        cinfo_s[j].port = cinfo_s[j+1].port;
        update_client_info(cinfo_s,m,j,nothing,TRUE);
    }
    m->cinfo[clients_online].score= -1; //mete ultima posição a -1(cliente removido)
    m->cinfo[clients_online].client_ID = 0;
    cinfo_s[clients_online].score = -1;
    cinfo_s[clients_online].client_ID = 0;
}

void inicializa_score(message *m ,client_info_s * cinfo_s)
{
    //score dos clientes todos a -1 (to avoid and force condition to print)
    for(int i = 0; i < MAX_CLIENTS; i++){
        m->cinfo[i].score=-1;
        cinfo_s[i].score=-1;
    }
}

void first_client_routine(message * m, ball_position_t * ball_s, client_info_s * cinfo_s){
    inicializa_score(m, cinfo_s); //inicializa todas as posições do score a 0
    place_ball_random( &m->ball_position, ball_s); // inicializa a posição da bola
    m->point = TRUE;
    m->msg_type = 2; //send _ball
    m->cinfo[0].paddle_position.y=WINDOW_SIZE -2;
    m->cinfo[0].paddle_position.x=WINDOW_SIZE/2;
}

int main()
{
    initialize_ncurses(); 
    //---------Var declaration-----------
    int sock_fd;
    int clients_online = 0;                 //counts online clients
    struct sockaddr_in local_addr;          //structure describing an internet socket address
    struct sockaddr_in client_addr;         
    client_info_s cinfo_s[MAX_CLIENTS];     //Structure that will save relevant info from message in server
    message m;                              //Message 
    ball_position_t ball_s;                 //Structure that will save current ball position in server
    
    int move_ball_counter =0;               //Var used to move the ball after n paddle_moves 
    
    create_socket(&sock_fd);

    /*-----------BIND SOCKET-----------*/
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(SOCK_PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    socklen_t client_addr_size = sizeof(struct sockaddr_in);
    bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
    /*---------------------------------*/

   
    WINDOW *my_win = newwin(5, WINDOW_SIZE, 0, 0); //Declares game window
    create_window(my_win);

    while (1)
    {
        
        recvfrom(sock_fd, &m, sizeof(message), 0,
                 (struct sockaddr *)&client_addr, &client_addr_size);
        
      switch (m.msg_type)
        {
        case 0: //m.msg_type = 0 -> Connect
            if ( clients_online == MAX_CLIENTS) //If the server is full
            { 
                m.msg_type = 4; //mgs_type=4 -> the server informs the client that it is full and will close it's window.
            }
            else
            {   
                clients_online ++;  //If the server isn't full, increment the number of online clients.
                if (clients_online == 1) //If its the first client to connect 
                { 
                    first_client_routine(&m,&ball_s,cinfo_s); //Prefroms a "first client" routine that initializes variables
                }
                add_client(&m, cinfo_s, clients_online, client_addr, ball_s); //Gives client an ID, initializes its message values, 
                mvwprintw(my_win, 2, 1, "%d connected at the moment", clients_online);
                wrefresh(my_win);
                
            }
            //Sends Board_update message  (msg_type = 3)
            sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&client_addr, client_addr_size);

            break;
         case 1: //Disconnect message
            remove_client(&m,cinfo_s,clients_online); 
            clients_online --;
            mvwprintw(my_win, 2, 1, "%d connected at the moment", clients_online);
            wrefresh(my_win);
            break; 

        case 2: //Paddle_move
            paddle_move(&m.cinfo[m.client_contacting].paddle_position,&cinfo_s[m.client_contacting].paddle_position_s);
            m.msg_type = 3;
            move_ball_counter ++;
            if ((move_ball_counter == clients_online) ||m.point){
                move_ball(&m, cinfo_s, clients_online, &ball_s);
                move_ball_counter = 0;
            }
            //update_message_info(cinfo_s, &m, clients_online, ball_s);
            for (int i = 0; i < clients_online; i++)
            {
                m.cinfo[i].client_ID = cinfo_s[i].client_ID;
                m.cinfo[i].paddle_position.length =cinfo_s[i].paddle_position_s.length;
                m.cinfo[i].paddle_position.x =cinfo_s[i].paddle_position_s.x;
                m.cinfo[i].paddle_position.y =cinfo_s[i].paddle_position_s.y;

            }
            for (int i = 0; i < MAX_CLIENTS; i++){
                m.cinfo[i].score = cinfo_s[i].score;
            }
            copy_ball_info(&m.ball_position,&ball_s);
            //m.cinfo->client_ID=cinfo_s->client_ID
            sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&client_addr, client_addr_size);
            /*for (int i = 0; i < clients_online; i++)// atenção verificar há coisas q vão mudar bastante agora..
            {
                    sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&client_addr, client_addr_size);
            }*/
            break;      
        }
    }
    close(sock_fd);
    endwin(); //End ncurses mode

    return 0;
}
