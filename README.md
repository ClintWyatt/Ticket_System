
Note: This program will only run in linux, and the 2 clients must be on different machines for this program to work
To run the program, follow the steps below:

1. Compile the server file (svr.c) by typing gcc svr.c
2. To run the server, type ./a.out <port number>, with <port number> being a number between 1,023 and 49,152
3. On another machine, compile the client file (ticketCli.c) by typing gcc ticketCli.c -lpthread.
4. To run the clinet, type ./a.out <server machine name> <server port> <other client's ip address>
5. Now, on another machine, compile the other client (ticketCli.c) by typing gcc ticketCli.c -lpthread.
6. To run the clinet, type ./a.out <server machine name> <server port> <other client's ip address>

To find the ip address on a machine running linux ubuntu, type ifconfig.  
