#include "header.h"

void criar_socket(int *sock_fd)
{
    *sock_fd = socket(AF_INET, SOCK_DGRAM, 0); //cria socket
    if (*sock_fd == -1)
    { //verifica se socket esta bem criada
        perror("socket: ");
        exit(-1);
    }
}

int new_paddle (message *m,  client_info_s  *cinfo_s, int adder){ 
    paddle_position simul_paddle; 
    simul_paddle.length=PADDLE_SIZE;
    simul_paddle.x = WINDOW_SIZE/2;
    if ((WINDOW_SIZE-2 + cinfo_s->clients_online)%2){ 
        simul_paddle.y = WINDOW_SIZE-2 + cinfo_s->clients_online+adder;
    }else{
        simul_paddle.y = -2+cinfo_s->clients_online; 
    }
    
    for(int i; i< (cinfo_s->clients_online); i++){
        if(simul_paddle.y == m->cinfo[i].paddle_position.y){  //AQUI PRECISA DA COORDENADA DO X ???

            return 1; // se encontrar um paddle dá return 1
        }
    }

    //se não encontrar paddle já criada, entao vai passar as cenas do paddle simulado para o cliente 
    m->cinfo[cinfo_s->clients_online-1].paddle_position.length = PADDLE_SIZE;
    m->cinfo[cinfo_s->clients_online-1].paddle_position.x=simul_paddle.x;
    m->cinfo[cinfo_s->clients_online-1].paddle_position.y=simul_paddle.y;
    return 0; 

}

void add_client(message  * m, struct client_info_s  *cinfo_s, struct sockaddr_in *client_addr)
{
    static int ID=0;
    ID++;
    int valid; 
    int adder=0; //se encontrar um paddle na posição em que cria esta variavel vai incrementar para aumentar o y 
    
    cinfo_s->clients_online++; //incrementa o nr de clients online guardado no server
    cinfo_s->client_ID_s[cinfo_s->clients_online-1]=ID; //Atribui ID ao cliente a ser criado 
     

    do{
        adder++;
        valid = new_paddle(&m, &cinfo_s, adder);
    }while(!valid);

    //guardar adress do client no server no server
    //TODO arranjar isto 
    clients_addr[client].sin_family = client_addr.sin_family;
    clients_addr[client].sin_addr = client_addr.sin_addr;
    clients_addr[client].sin_port = client_addr.sin_port;
    m->msg_type = 0; //connect message
   
}

void place_ball_random(ball_position_t *ball)
{
    ball->x = rand() % WINDOW_SIZE;
    ball->y = rand() % WINDOW_SIZE;
    ball->c = 'o';
    ball->up_hor_down = rand() % 3 - 1;    //  -1 up, 1 - down
    ball->left_ver_right = rand() % 3 - 1; // 0 vertical, -1 left, 1 right
}

void create_window(WINDOW *my_win)
{
    // creates a window and draws a border
    box(my_win, 0, 0);
    wrefresh(my_win);
    keypad(my_win, true);
}

void initialize_ncurses()
{
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
}

void remove_client(struct sockaddr_in *clients_address, int removed, int client, message *m)
{
    //ciclo que procura o address do gajo que se está a disconectar
    for (int j = removed; j < client; j++)
    {
        m->score[j][1] = m->score[j + 1][1];
        m->score[j][2] = m->score[j + 1][2];
        clients_address[j].sin_addr = clients_address[j + 1].sin_addr;
    }
    m->score[client][1] = -1; //mete ultima posição a -1(cliente removido)
    m->score[client][2] = -1;
}

void inicializa_score(message *m )
{
    //score dos clientes todos a 0 (to avoid trash)
    for(int i =0; i<= MAX_CLIENTS; i++){
        m->cinfo[i].score=0;
    }
}

void first_client_routine(message * m, int * active_client, int * check_for_new_client){
    inicializa_score(&m); //inicializa todas as posições do score a 0
    place_ball_random( &m->ball_position); // inicializa a posição da bola
    m->point = TRUE;
    m->msg_type = 2; //send _ball
    *active_client = 0;
    *check_for_new_client = 0;
}

void update_client_info(client_info_s * clients_info,message * m){
    clients_info[m->client_contacting].score =m->score[m->client_contacting][1];
    clients_info[m->client_contacting].client_number =m->score[m->client_contacting][2];
    //clients_info[m->client_contacting].paddle_position.length = m->paddle_position->length;
    clients_info[m->client_contacting].paddle_position.x = m->paddle_position->x;
    clients_info[m->client_contacting].paddle_position.y = m->paddle_position->y;
    //clients_info[m->client_contacting]. = m->paddle_position->length;
}

int main()
{
    initialize_ncurses();
    int sock_fd;
    int check_for_new_client = 0;  //verifica se se trata de um novo utilizador para
    int created_clients = 0;                //(incrementa quando um client novo é criado)
    int active_client;             //(qual é q é o client que está play state)
    struct sockaddr_in local_addr; //structure describing an internet socket address
    client_info_s cinfo_s; 
    message m;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(struct sockaddr_in);
    time_t start,end;
    int timer;

    cinfo_s.clients_online=0;
    criar_socket(&sock_fd);

    /*-----------BIND SOCKET-----------*/
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = ntons(SOCK_PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
    /*---------------------------------*/

    /* creates a window and draws a border */
    WINDOW *my_win = newwin(10, WINDOW_SIZE, 0, 0);
    create_window(my_win);
    while (1)
    {
  
        recvfrom(sock_fd, &m, sizeof(message), 0,
                 (struct sockaddr *)&client_addr, &client_addr_size);
      switch (m.msg_type)
        {
        case 0: // msg type = 0 -> new client (connect)
            if ( cinfo_s.clients_online == MAX_CLIENTS) // FULL_SERVER
            { 
                m.msg_type = 4;
            }
            else
            {
                add_client(&m, &cinfo_s, &client_addr);
                
                if (cinfo_s.clients_online == 1)
                { //STEP C If this client is the first one to connect,the server sends as reply a Send_ball message (step C).
                    first_client_routine(&m,&active_client,&check_for_new_client);
                    time (&start);
                }
                board_update();
                 
                mvwprintw(my_win, 2, 1, "%d connected at the moment", cinfo_s.clients_online);
                wrefresh(my_win);
            }
            sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&client_addr, client_addr_size);

            break;
        case 2: /*paddle_move*/
            // 1- O cliente vai mandar uma mensagem do tipo Paddle Move.
            // 2- O server var processar a info que o cliente manda e manda o board_update();
            if (m.point){
                //m.score[m.client_contacting][1]++;------- ver se atualiza logo tbm para o client em questão
                clients_info[m.client_contacting].score ++;
            }
            update_client_info(clients_info,&m);// fiquei aqui mas cansadissimo rever tudo para cima basicly
            
            m.msg_type = 3;
            for (int i = 0; i < cinfo_s.clients_online; i++)// atenção verificar há coisas q vão mudar bastante agora..
            {
                    sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&clients_info[i].client_address, client_addr_size);
            }
            m.point = FALSE;
            //choose_next_client();
            if (active_client + 1 == cinfo_s.clients_online)
                active_client = -1;
            active_client++;
            m.allow_release = FALSE;
            time (&start);
            m.msg_type = 2; //next client enters Play State
            /*send_ball();*/ sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&clients_info[active_client].client_address, client_addr_size);
            break;

        case 3:
            /*When the server receives the Move_ball message, it must forward the same ball
                movement to all other connected clients (step L), so that every client can update the
                corresponding screen to show the ball in the new position.*/
            //MOVE_BALL (STEP L)
            //Updates ball position to all clients that aren't in Play State
            if (m.point)
                m.score[active_client][1]++;
            m.allow_release =FALSE;
            while (check_for_new_client != cinfo_s.clients_online)
            {
                m.score[check_for_new_client][1] = 0;
                m.score[check_for_new_client][2] = created_clients;
                check_for_new_client++;
                m.point = TRUE;//Forces score to
            }
            m.msg_type = 3;
            for (int i = 0; i < cinfo_s.clients_online; i++)
            {
                if (i != active_client)
                    sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&clients_info[i].client_address, client_addr_size);
            }
            m.msg_type = 2;
            time (&end);
            timer = difftime(end,start);
            if (timer >= 10) m.allow_release =TRUE;
            sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&clients_info[active_client].client_address, client_addr_size);
            break;
        case 1:
            //remove_client(clients_address , active_client, client, &m); //remove cliente e limpa os seus dados

            for (int j = active_client; j < cinfo_s.clients_online; j++)
            { //shift left
                m.score[j][1] = m.score[j + 1];
                m.score[j][2] = m.score[j + 1];
                clients_info[j].client_address.sin_addr.s_addr = clients_info[j+1].client_address.sin_addr.s_addr;
                clients_info[j].client_address.sin_family = clients_info[j+1].client_address.sin_family;
                clients_info[j].client_address.sin_port = clients_info[j+1].client_address.sin_port;
                //clients_address[j].sin_zero=clients_address[j+1].sin_zero;
            }
            m.score[cinfo_s.clients_online][1] = -1; //mete ultima posição a -1(cliente removido)
            m.score[cinfo_s.clients_online][2] = -1;
            cinfo_s.clients_online--;
            check_for_new_client--;
            for (int i = 0; i < cinfo_s.clients_online; i++)
            { // updatescoreboard for players
                sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&clients_info[i].client_address, client_addr_size);
            }
            m.msg_type = 2;
            if (active_client + 1 == cinfo_s.clients_online)
                active_client = 0;
            sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&clients_info[active_client].client_address, client_addr_size);
            break;
        }
    }

    endwin(); /* End curses mode		  */

    return 0;
}
