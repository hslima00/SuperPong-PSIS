#include "remote-char.h"

void criar_socket(int *sock_fd)
{
    *sock_fd = socket(AF_INET, SOCK_DGRAM, 0); //cria socket
    if (*sock_fd == -1)
    { //verifica se socket esta bem criada
        perror("socket: ");
        exit(-1);
    }
}

void add_client(struct sockaddr_in *clients_addr, int client, struct sockaddr_in client_addr , message  * m)
{
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
        m->score[j] = m->score[j + 1];
        clients_address[j].sin_addr = clients_address[j + 1].sin_addr;
    }
    m->score[client] = -1; //mete ultima posição a -1(cliente removido)
}

void inicializa_score(int score[])
{
    for (int i = 1; i <= MAX_CLIENTS; i++)
    {
        score[i] = -1;
    }
    score[0] = 0;
}



int main()
{
    initialize_ncurses();
    int sock_fd;
    int check_for_new_client = 0;  //verifica se se trata de um novo utilizador para
    int client = 0;                //(incrementa quando um client novo é criado)
    int active_client;             //(qual é q é o client que está play state)
    struct sockaddr_in local_addr; //structure describing an internet socket address
    struct sockaddr_in client_addr;
    message m;
    socklen_t client_addr_size = sizeof(struct sockaddr_un);
    time_t start,end;
    int timer;
    // 6.2 Step 2 (server.c)
    struct sockaddr_in clients_address[MAX_CLIENTS]; //array que guarda ID dos clients
    //--//
    criar_socket(&sock_fd);

    /*-----------BIND SOCKET-----------*/
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(SOCK_PORT);
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
        int switcher = m.msg_type;
        switch (switcher)
        {
        case 0: // msg type = 0 -> new client (connect)
            if (client == MAX_CLIENTS)
            { // FULL_SERVER
                m.msg_type = 5;
            }
            else
            {
                add_client(clients_address, client, client_addr, &m);
                if (client == 0)
                { //STEP C If this client is the first one to connect,the server sends as reply a Send_ball message (step C).
                    
                    inicializa_score(m.score);           //inicializa todas as posições do score a -1
                    place_ball_random(&m.ball_position); // inicializa a posição da bola
                    m.allow_release =FALSE;
                    m.point = TRUE;
                    m.msg_type = 2; //send _ball
                    active_client = 0;
                    check_for_new_client = 0;
                    time (&start);
                }
                client++;
                mvwprintw(my_win, 2, 1, "%d connected at the moment", client);
                wrefresh(my_win);
            }
            sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&client_addr, client_addr_size);

            break;
        case 1: /*release_ball*/
            if (m.point)
                m.score[active_client]++;
            m.msg_type = 3;
            for (int i = 0; i < client; i++)
            {
                    sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&clients_address[i], client_addr_size);
            }
            m.point = FALSE;
            //choose_next_client();
            if (active_client + 1 == client)
                active_client = -1;
            active_client++;
            m.allow_release = FALSE;
            time (&start);
            m.msg_type = 2; //next client enters Play State
            /*send_ball();*/ sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&clients_address[active_client], client_addr_size);
            break;

        case 3:
            /*When the server receives the Move_ball message, it must forward the same ball
                movement to all other connected clients (step L), so that every client can update the
                corresponding screen to show the ball in the new position.*/
            //MOVE_BALL (STEP L)
            //Updates ball position to all clients that aren't in Play State
            if (m.point)
                m.score[active_client]++;
            m.allow_release =FALSE;
            while (check_for_new_client != client)
            {
                m.score[check_for_new_client] = 0;
                check_for_new_client++;
                m.point = TRUE;//Forces score to
            }
            m.msg_type = 3;
            for (int i = 0; i < client; i++)
            {
                if (i != active_client)
                    sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&clients_address[i], client_addr_size);
            }
            m.msg_type = 2;
            time (&end);
            timer = difftime(end,start);
            if (timer >= 10) m.allow_release =TRUE;
            sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&clients_address[active_client], client_addr_size);
            break;
        case 4:
            //remove_client(clients_address , active_client, client, &m); //remove cliente e limpa os seus dados

            for (int j = active_client; j < client; j++)
            { //shift left
                m.score[j] = m.score[j + 1];
                clients_address[j].sin_addr.s_addr = clients_address[j + 1].sin_addr.s_addr;
                clients_address[j].sin_family = clients_address[j + 1].sin_family;
                clients_address[j].sin_port = clients_address[j + 1].sin_port;
                //clients_address[j].sin_zero=clients_address[j+1].sin_zero;
            }
            m.score[client] = -1; //mete ultima posição a -1(cliente removido)
            client--;
            check_for_new_client--;
            for (int i = 0; i < client; i++)
            { // updatescoreboard for players
                sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&clients_address[i], client_addr_size);
            }
            m.msg_type = 2;
            if (active_client + 1 == client)
                active_client = 0;
            sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&clients_address[active_client], client_addr_size);
            break;
        }
    }

    endwin(); /* End curses mode		  */

    return 0;
}
