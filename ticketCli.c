#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h> 
#include <errno.h>
#define MAXLINE 4096

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t signal = PTHREAD_COND_INITIALIZER;//used to signal other threads
void *client();
void *svr(); //prototype for the server thread
pthread_t svr_tid, cli_tid; //thread for the server and client
int sockfd, sockfd1, sockfd2, portno, terminate; 
int ticketNum[15] = {0 ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //array of ticket numbers
int ticketPrice[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //array for the price of each ticket
int balance = 4000; //balance of the client. Used to buy tickets
struct sockaddr_in serv_addr, scalp_addr;//settin up ip adderss structures
int shared = 0;
char buffer[256];
socklen_t len1;
int main(int argc, char **argv)
{
	//struct sockaddr_in serv_addr, scalp_addr;//settin up ip adderss structures
	struct hostent *server;
	//socklen_t len1; 
	//char buffer[256];
	
	//tcp setup
	if (argc != 4)//the 4th argument ip address will likely need to be set to the struct sockaddr_in servaddr data structure in the client thread
	{
        	fprintf(stderr,"usage: %s <hostname> <port> <other client machine IP address>\n", argv[0]);
        	exit(EXIT_FAILURE);
    	}
	portno = atoi(argv[2]);//getting the port number
	pthread_create (&svr_tid, NULL, (void *) &svr, NULL);//starting the udp connection for this client, acts as a server.
	
	//setting up the server information for tcp
	sockfd = socket(AF_INET, SOCK_STREAM, 0);//creating the socket
	if(sockfd <0)//if socket creation fails
	{
		perror("socket failure");
		exit(EXIT_FAILURE);
	}

	server = gethostbyname(argv[1]);//geting the server name
	if(server == NULL)
	{
		perror("ERROR! host does not exist");
		exit(EXIT_FAILURE);
	}
	
	bzero((char *) &serv_addr, sizeof(serv_addr));//clear out the data on serv_addr
	serv_addr.sin_family = AF_INET;//set connection type to internet
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);
	
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
	{
		perror("ERROR connecting");
	}
	//tcp setup end
	
	

	bzero(&scalp_addr, sizeof(scalp_addr));
	scalp_addr.sin_family = AF_INET; 
	scalp_addr.sin_port = htons(portno +1);
	inet_pton(AF_INET, argv[3], &scalp_addr.sin_addr);//setting up the ip address from the other machine
	sockfd1 = socket(AF_INET, SOCK_DGRAM, 0); //udp connection
	if(sockfd1 <0)
	{
		perror("ERROR: udp socket failure");
		exit(EXIT_FAILURE);
	}
	len1 = sizeof(scalp_addr);
	pthread_create (&cli_tid, NULL, (void *) &client, NULL);//creating the client thread
 
	int i, n, j, sum, _ticket, _price;
	char ticket_num[10];
	//char _price[10];
	char *token;
	for(i =0; i<15; i++)
	{
		printf("\033[0;33m");//changing output to blue
		printf("[<>CLIENT]: BUY %d \n", balance);
		usleep(1000);
		sprintf(buffer, "%d", balance);
		send(sockfd, buffer, sizeof(buffer), 0); //sending a buy request through tcp socket
		recv(sockfd, buffer, sizeof(buffer), 0);//recieving the message back from the server
		
		if(buffer[0] != '0')//if the ticket was bought
		{

			token = strtok(buffer, " ");
			balance = atoi(token);
			token = strtok(NULL, " ");
			ticketNum[i] = atoi(token);
			token = strtok(NULL, " ");
			ticketPrice[i] = atoi(token);
			printf("\033[0;32m");//changing the output to green
			printf("[<-SERVER]: Buy %d $%d ok\n", ticketNum[i], ticketPrice[i]);
		}
		else //if server is out of tickets or the balance is not enough
		{
			pthread_cond_signal(&signal); //signaling the client thread to buy from the scalper
			usleep(1000);
		}
		memset(buffer, 0, sizeof(buffer));//resetting the buffer
	}
		
	printf("\033[0m"); //resetting the color to the default color
	
	int z;
	/*
	for(z =0; z < 2147483647; z++)//this loop will simulate a sleep. Sleep is not used here since asleep also puts the threads to sleep as well
	{
		//do nothing in this loop	
	}
	
	*/
	sleep(3);
	shared = 98;
	pthread_mutex_destroy(&mutex1);
	pthread_cond_signal(&signal);//ending the client thread
	pthread_join(cli_tid, NULL);//wait for clientthread to finish
	
	sendto(sockfd1, "Done", 4, 0, (const struct sockaddr *) &scalp_addr, sizeof(scalp_addr));//telling the other client to end their server thread
	pthread_mutex_destroy(&mutex);
	pthread_join(svr_tid, NULL);//waiting for thread to end
	 //closing udp client connection
	sleep(1);
	
	//close(sockfd2);	
	 printf("\033[0m"); //resetting the color to the default color

	printf("Ticket# price) \n");
        for(z =0; z < 15; z++)
        {
                printf("%d \t %d \n", ticketNum[z], ticketPrice[z]);
        }
	close(sockfd);
	close(sockfd1);
	return 0;

}

void *client()
{
	int n, j, sum, _ticket, _price;	
	shared = 0;

	while(shared != -1)
	{
		while(shared ==0)
		{
			pthread_cond_wait(&signal, &mutex);
			shared++;
		}
		if(shared == 99)
		{
			break;//end the thread by breaking the loop
		}
		sprintf(buffer, "%d", balance);
                        //len1 = sizeof(scalp_addr);
                sendto(sockfd1, buffer, sizeof(buffer), 0, (const struct sockaddr *) &scalp_addr, sizeof(scalp_addr));//sending a buy request to the scalper

                n = recvfrom(sockfd1, buffer, sizeof(buffer), 0, (struct sockaddr *) &scalp_addr, &len1); //recieving message back from the scalper
		//printf("message from scalper %s \n", buffer);
                if(buffer[0] == 'N')//if buyback failed from scalper, sell ticket back to the server
                {
                       printf("\033[0;31m"); //changing the output to red
                       printf("[SCALPER ->]: not enough money\n");
                       for(j =0; j < 15; j++)
                       {
                               if(ticketNum[j] !=0)//if the client has a ticket to sell back to the server
                               {
                                   break;
                               }
                       }
                       memset(buffer, 0, sizeof(buffer));
                       sprintf(buffer, "%d", ticketNum[j]);
                       sum = ticketPrice[j];//getting the sum of the ticket
                       ticketNum[j] = 0;
                       ticketPrice[j] =0;
                       balance +=sum; //adding funds to the client
                       send(sockfd, buffer, sizeof(buffer), 0); //sending ticket number back to the server
                  }
                  else //buybck suceeded
                  {
                 	 //n = recvfrom(sockfd1, buffer, sizeof(buffer), 0, (struct sockaddr *) &scalp_addr, &len1);
                 	 char *token = strtok(buffer, " ");//setting up string tokenizing
                         balance = atoi(token);//getting the updated balance
                         token = strtok(NULL, " ");//going to the next non space character
                         _ticket = atoi(token);//getting ticket
                         token = strtok(NULL, " ");//getting the next no space character
                         _price = atoi(token);//getting the price
                         for(j =0; j< 15; j++)
                         {
                                 if(ticketNum[j] ==0)//if there is a open slot for the ticket
                                 {
                                         ticketNum[j] = _ticket;
                                         ticketPrice[j] = _price;
                                        break;
                                 }

                         }
                          send(sockfd, "0", 1, 0);//telling the server that buying from the scaper succeeded

                  }
         
                memset(buffer, 0, sizeof(buffer));//resetting the buffer
		shared = 0;
	}

	
	pthread_mutex_unlock(&mutex1);
	pthread_exit(NULL);

}

void *svr() //server thread
{
	pthread_mutex_lock(&mutex);
	int port, n; //socket and port number
	struct sockaddr_in cliaddr, Serv_addr;
	sockfd2 = socket(AF_INET, SOCK_DGRAM, 0);
	char message[50];
	int amount, lowest, number;
       	int index =0; 
	char ticket[10];
	char price [10];
	char _total[10];
	int _price;
	int i; //used to find the cheapest ticket	
	if(sockfd2 <0)
	{
		perror("ERROR: upd connection in server thread failed");
	}
	port = portno +1;//incrementing the port number
	bzero(&Serv_addr, sizeof(Serv_addr));//clearing memory of Serv_addr
	Serv_addr.sin_family = AF_INET;//setting up connection type
	Serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);//setting ip address to this machine
	Serv_addr.sin_port = htons(port);//setting up the port
	bind(sockfd2, (struct sockaddr *) &Serv_addr, sizeof(Serv_addr));
	terminate = 0;
	socklen_t len;
	len = sizeof(cliaddr);
	
	while(terminate != 99)
	{
		n = recvfrom(sockfd2, message, sizeof(message), 0, (struct sockaddr *) &cliaddr, &len); //recieving mesage from client
		if(message[0] == 'D')
		{
			break; //end the thread by breaking this loop
		}
		len = sizeof(cliaddr);
		printf("\033[0;35m"); //changing the color to magenta
		printf("[<-BUYER] -> SCALP %s \n", message);
		amount = atoi(message);//getting the integer amount from the client
		lowest = 401; //greater than the maximum ticket price
		for( i =0; i < 15; i++)
		{
			if(ticketPrice[i]+100 < lowest && ticketPrice[i] !=0)
			{
				lowest = ticketPrice[i]+100; //resetting the lowest
				index = i;//setting the index for lowest ticket
			}	
		}
		//lowest = ticketPrice[index] +10;
		memset(message, 0, sizeof(message)); //resetting the message string
		if(amount < lowest)//if the client cant buy from the scalper
		{
			sendto(sockfd2, "N", 1, 0, (const struct sockaddr *) &cliaddr, len); 
			memset(message, 0, sizeof(message));//resetting the message string
		}
		else//ticket can be sold
		{
			printf("\033[0;31m");//changing output to red
			printf("[->SCALPER]: SCALP %d %d \n", ticketNum[index], lowest);
			amount -= lowest;//updating the clinets balance
			_price = lowest;//getting the ticket price
			number = ticketNum[index];//getting the ticket number
			ticketNum[index] =0;
			ticketPrice[index] =0;
			sprintf(_total, "%d", amount);//converting the balance to a string
			sprintf(ticket, "%d", number);//converting to string
			sprintf(price, "%d", _price);//converting to string
			memcpy(message, _total, sizeof(_total));//adding string to bigger string
			strcat(message, " ");//adding a space
			strcat(message, ticket);//adding the ticket number to the string
			strcat(message, " ");//adding a space to be used with strtok
			strcat(message, price);//adding the ticket price to the string

			/*for some reason the first packet is always dropped when sending back to the client. Not sure on how to solve this issue yet...*/			
			sendto(sockfd2, message, sizeof(message), 0, (const struct sockaddr *) &cliaddr, len);//sending the price and ticket back to the cleint
			memset(message, 0, sizeof(message)); //resetting the message string
			
		}
		
	}
	
	pthread_mutex_unlock(&mutex);
	pthread_exit(NULL);//terminating the thread
}
