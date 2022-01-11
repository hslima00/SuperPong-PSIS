#include "remote-char.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ncurses.h>


#define WINDOW_SIZE 15



int main()
{	
    int SIZE=50;
    char adress_keyboard[SIZE];
    int sock_fd;
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1){
        perror("socket: ");
        exit(-1);
    } 

    printf("IP do servidor? \n");
    fgets(adress_keyboard, SIZE , stdin);// gets adress from user
    adress_keyboard[strlen(adress_keyboard)-1] = '\0';
          

	initscr();		    	
	cbreak();				
    keypad(stdscr, TRUE);   
	noecho();			    

    /* creates a window and draws a border */
    WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0 , 0);	
	wrefresh(my_win);

    int ch;
    int pos_x;
    int pos_y;
    //int n_bytes;

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SOCK_PORT);
    if( inet_pton(AF_INET, adress_keyboard, &server_addr.sin_addr) < 1){
		printf("no valid address: \n");
		exit(-1);
	}

    message m;
    m.msg_type = 5;
    sendto(sock_fd, &m, sizeof(message), 0, 
            (const struct sockaddr *)&server_addr, sizeof(server_addr));
    
    
    while (1){
        recv(sock_fd, &m, sizeof(message), 0);
        
        if(m.msg_type == 6){
            /*deletes old place */
            wmove(my_win, pos_x, pos_y);
            waddch(my_win,' ');
            pos_x = m.ball_position.x;
            pos_y = m.ball_position.y;
            ch = m.ball_position.c;
            wmove(my_win, pos_x, pos_y);
            waddch(my_win,ch| A_BOLD);
            wrefresh(my_win);           			
        }
    }
  	endwin();			/* End curses mode		  */

	return 0;
}