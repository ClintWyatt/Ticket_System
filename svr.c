#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <time.h>

int main(int argc, char **argv)
{
	int sockfd, portno;
	int ns;		//first connection
	int ns2;	//second connection
	int pos; 	//used to loop through the ticket status
	int found =0;	//used to confirm if a ticket is available
	char buffer[256];
	fd_set fds;
	int clilen, clilen2, maxfd, nready;
	struct sockaddr_in servaddr, cli_addr, cli2_addr;//server and clients
	char *client1HostName; //the host name of the first client
	char *client2Hostname; //the host name of the second client
	int ticketArray[25] = {10000, 10001, 10002, 10003, 10004, 10005, 10006, 10007, 10008, 10009, 10010, 10011, 
			10012, 10013, 10014, 10015, 10016, 10017, 10018, 10019,  10020, 10021, 10022, 10023, 10024};
	//Cost of Ticket
    	int ticketCost[25] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	int ticketStatus[25] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

	srand(time(NULL));
	//printf("hi \n");
	int i =0;
   	for( i; i < 25; i++)
	{
        	ticketCost[i] = rand()%200 + 150;//Randomize cost
    	}
   	i = 0;
	/*printing the ticket array*/
	printf("TICKET NUMBER    PRICE   STATUS\n");
	printf("-------------------------------\n");
	for(i; i < 25; i++)
	{
		printf("[Tkt# %d]: \t $ %d \t AVAIL\n", ticketArray[i], ticketCost[i]);
	}

	/* TCP setup  */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd <0)
	{
		perror("ERROR opening socket");
	}
	/*setting up the server */	
	bzero((char *) &servaddr, sizeof(servaddr));//similar to memcpy
	portno = atoi(argv[1]); //the port is equal to the string to integer of the second command line argument
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(portno);
	if(bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) <0)
	{
		perror("ERROR on binding");
		exit(1);
	}
       	printf("0 clients connected. Waiting on CLIENT 1 to connect: \n");
	/*Listening for connections*/
	if(listen(sockfd, 5) <0)
	{
		perror("listen");
		exit(1);
	}

	clilen = sizeof(cli_addr);

	/*Accept client 1 connection*/
    	if((ns = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) <0)
	{

		perror("accept");
		exit(1);
	}
	

    	printf("1 client connected. Waiting on CLIENT 2 to connect: \n");
	clilen2 = sizeof(cli2_addr);

	/*Accept client 2 connection */
	if((ns2 = accept(sockfd, (struct sockaddr *) &cli2_addr, &clilen2)) <0){
		
		perror("accept");
		exit(1);
	}

	printf("2 clients connected. Ready for incomming requests....\n");
	maxfd = (ns >ns2 ? ns : ns2) +1; //setting the max file descriptor to be used with select
	char price[10];
	char tcket[10];
	int tickNum, balance;
	int a =0, z =0;

	for(a =0; a < 30; a++)
	{
		FD_ZERO(&fds);//clearing the file descriptors
		FD_SET(ns,&fds);
		FD_SET(ns2, &fds);
		nready = select(maxfd, &fds, (fd_set *) 0, (fd_set *) 0, (struct timeval *) 0); 
		if( FD_ISSET(ns, &fds)) //if activity from client 1
		{
			recv(ns, buffer, sizeof(buffer),0); //recieving the balance from the client
			balance = atoi(buffer); //turning the string of the balance into a integer
			memset(buffer, 0, sizeof(buffer)); //resetting the buffer
			printf("\033[0;33m"); //setting the output color to blue
			printf("[<-CLIENT 1]: buy %d \n", balance);//printing client balance
			for(pos =0; pos < 25; pos++)
			{
				if(ticketStatus[pos] == 1) //if ticket is available
				{
					found =1; //setting found to 1
					break; //exit the loop
				}
			}
			if(balance > ticketCost[pos] && found ==1)//if the client can buy a ticket
			{
				printf("\033[0;32m");//changing the output to green
				printf("[-> SERVER ]: CLIENT 1 buy %d for $%d successful \n", ticketArray[pos], ticketCost[pos]);
				ticketStatus[pos] = 0;//ticket is now sold
				balance -= ticketCost[pos];//update the balance
				sprintf(buffer, "%d", balance); //converting the integer of the balance to a string
				sprintf(price, "%d", ticketCost[pos]);//converting ticket price to a string
				sprintf(tcket, "%d", ticketArray[pos]);//converting ticket number to a string
				strcat(buffer, " ");
				strcat(buffer, tcket);//adding the ticket number to the buffer
				strcat(buffer, " ");
				strcat(buffer, price);//adding the ticket price to the buffer
				send(ns, buffer, sizeof(buffer), 0);//sending all the ticket information to the client
			}
			else if(balance < ticketCost[pos])//the balance 
			{
				printf("\033[0;32m");//changing the output color to green
				printf("[-> SERVER ]: No Funds \n");
				memset(buffer, 0, sizeof(buffer));//resetting the string buffer
				strcat(buffer, "0");
				send(ns, buffer, sizeof(buffer), 0);//sending 0 back to the client
				recv(ns, buffer, sizeof(buffer), 0);
				if(buffer[0] != '0')//client did not sell to the scalper
				{
					tickNum = atoi(buffer);
						
					for(z=0; z < 25; z++)
					{
						if(ticketArray[z] == tickNum)
						{
							ticketStatus[z] =1;
							break;
						}
					}
					printf("\033[0;32m");
					printf("[->SERVER]: Buy from CLIENT1: %d  %d  ok\n", ticketArray[z], ticketCost[z]);

				}
			}
			else if(found == 0) //case tickets are sold out
			{
				printf("\033[0;32m");//changing the output to green
				printf("[-> SERVER ]: Tickets Sold Out \n");
				memset(buffer, 0, sizeof(buffer));//resetting the string buffer
				strcat(buffer, "0");//adding zero to the buffer
				send(ns, buffer, sizeof(buffer), 0);//sending 0 back to the client to tell it there are no tickets left
				recv(ns, buffer, sizeof(buffer), 0);//reciving a response from the client whether it could buy from the scalper
				if(buffer[0] != '0')//client did not buy from the scalper. buffer then contains the ticket number
				{
					tickNum = atoi(buffer);//getting the ticket number from the client	
					for(z=0; z < 25; z++)
					{
						if(ticketArray[z] == tickNum)//if tickets match
						{
							ticketStatus[z] =1;
							break;
						}
					}
					printf("[->SERVER]: Buy from CLIENT1: %d  %d  ok\n", ticketArray[z], ticketCost[z]);
				}

			}
		}
		found =0;
		if( FD_ISSET(ns2, &fds)) //if activity from client 2
		{
			recv(ns2, buffer, sizeof(buffer),0); //recieving the balance from the client
			balance = atoi(buffer); //turning the string of the balance into a integer
			memset(buffer, 0, sizeof(buffer)); //resetting the buffer
			printf("\033[0;33m"); //setting the output color to blue
			printf("[<-CLIENT 2]: buy %d \n", balance);
			for(pos =0; pos < 25; pos++)
			{
				if(ticketStatus[pos] == 1) //if ticket is available
				{
					found =1; //setting found to 1
					break; //exit the loop
				}
			}
			if(balance > ticketCost[pos] && found ==1)//if the client can buy a ticket
			{
				printf("\033[0;32m"); //changing the output to green
				printf("[-> SERVER ]: CLIENT 2 buy %d for $%d successful \n", ticketArray[pos], ticketCost[pos]);
				ticketStatus[pos] = 0;//ticket is now sold
				balance -= ticketCost[pos];
				sprintf(buffer, "%d", balance); //converting the integer of the balance to a string
				sprintf(price, "%d", ticketCost[pos]);//converting ticket price to a string
				sprintf(tcket, "%d", ticketArray[pos]);//converting ticket number to a string
				strcat(buffer, " ");
				strcat(buffer, tcket);//adding the ticket number to the buffer
				strcat(buffer, " ");
				strcat(buffer, price);//adding the ticket price to the buffer
				send(ns2, buffer, sizeof(buffer), 0);//sending all the ticket information to the client
			}
			else if(balance < ticketCost[pos])
			{
				printf("\033[0;32m"); //changing the output to green
				printf("[-> SERVER ]: No Funds \n");
				memset(buffer, 0, sizeof(buffer));//resetting the string
				strcat(buffer, "0");//adding a zero to the string 
				send(ns2, buffer, sizeof(buffer), 0);//sending 0 back to the client
				recv(ns2, buffer, sizeof(buffer), 0);
				if(buffer[0] != '0')//client did not sell to the scalper
				{
					tickNum = atoi(buffer);
						
					for(z=0; z < 25; z++)
					{
						if(ticketArray[z] == tickNum)
						{
							ticketStatus[z] =1;
							break;
						}
					}
					printf("[->SERVER]: Buy from CLIENT2: %d  %d  ok\n", ticketArray[z], ticketCost[z]);
				}
			}
			else if(found == 0) //case tickets are sold out
			{
				printf("\033[0;32m"); //changing the output to green
				printf("[-> SERVER ]: Tickets Sold Out \n");
				memset(buffer, 0, sizeof(buffer));
				strcat(buffer, "0");
				send(ns2, buffer, sizeof(buffer), 0);//sending 0 back to the client
				recv(ns2, buffer, sizeof(buffer), 0);
				if(buffer[0] != '0')//client did not sell to the scalper
				{
					tickNum = atoi(buffer);
						
					for(z=0; z < 25; z++)
					{
						if(ticketArray[z] == tickNum)
						{
							ticketStatus[z] =1;
							break;
						}
					}
					printf("[->SERVER]: Buy from CLIENT2: %d  %d  ok\n", ticketArray[z], ticketCost[z]);
				}

			}
		}
		found =0;
	}
	printf("\033[0m"); //resetting the color to the default color
	sleep(4);
	/*printing the final status of the server tickets*/
	
	printf("TICKET NUMBER    PRICE   STATUS\n");
	printf("-------------------------------\n");
	for(pos=0; pos <25; pos++){

		printf("[Tkt# %d]: \t $ %d \t", ticketArray[pos], ticketCost[pos]);
		/*if ticket was sold and not bought back*/
		if(ticketStatus[pos] == 0){
		printf(" SOLD\n");
		}
		/* case that the ticket is not sold*/
		else{
		printf(" AVAIL\n");
		}
	}
	//closing connections
	close(ns2);
	close(ns);
	close(sockfd);
	return 0;
}


