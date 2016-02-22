#include "server.h"


void * StartServer(void * p);
void * ClientHandler(void * c);

Database server_database;
Clients *server_clients;
//client *ClientInit(SOCKET clientSocket, DWORD id);

int main(void) {
  void *rv; /// WHY DOING THIS

	pthread_t server_thread;
	pthread_create(&server_thread, NULL, StartServer, NULL);
  pthread_join(server_thread, &rv);
}

void * StartServer(void * p)
{
	printf("Server\n");

	pthread_t server_threads[MAX_CLIENTS];

	
	server_clients = new Clients();

	int ListenSocket = -1;

  struct sockaddr_in addr;

  ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
  memset(&addr, '0', sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //
  addr.sin_port = htons(27012);

  bind(ListenSocket, (struct sockaddr *)&addr, sizeof(addr));
  if (listen(ListenSocket, 20) == -1) {
     cout << "Listen Error\n";
     exit(0);
  }



	int num_clients = 0;
	int client_socket;

	while (num_clients < MAX_CLIENTS) {
    
		client_socket = accept(ListenSocket, NULL, NULL);
		pthread_t client_thread;
		cout << "WE GOT ANOTHER" << endl;
		Client *client;
		client = new Client(client_thread, client_socket);

		pthread_create(&client_thread, NULL, ClientHandler, (void *)client);
		// add a line in case creation doesn't work?
		
		if (server_clients->new_client(client) != 0) {
			cout << "WHAT HAPPENED" << endl;
		} 
		server_threads[num_clients] = client_thread;

		num_clients++;
	}

  int i;
  for( i = 0; i < num_clients; i++) {
    pthread_join(server_threads[i], NULL);
  }

	exit(0);
}

void * ClientHandler(void * c)
{
	Client *client;
	client = (Client *)c;
	int i = 0;

	int recvResult, sendResult;
	char recvbuf[BUFFER_LEN];
	char sendbuf[BUFFER_LEN];
	int recvbuflen = BUFFER_LEN;
	int client_socket = client->get_client_socket();

	do {

		recvResult = recv(client_socket, recvbuf, recvbuflen, 0);
		if (recvResult > 0) {
			cout << "RECIEVED: " << recvbuf << endl;
			memcpy(sendbuf, ParseClientString(recvbuf, client), BUFFER_LEN);
			if ( i > 0) cout << "CONNECTED AS:" << client->is_logged_in() << endl;

			cout << "sendbuf: " << sendbuf << endl;
			i++;
			sendResult = send(client_socket, sendbuf, recvResult, 0);
			if (sendResult < 0) {
				cout << "Send failure!\n";
				close(client_socket);
				exit(0);
			}
		}
		else if (recvResult == 0) {
			cout << "Losing a client\n";
      logoutUser(client);
      close(client_socket);
    }
		else {
			cout << "recv failure\n";
			close(client_socket);
		}

	} while (recvResult > 0);

}



/*
	struct addrinfo *result = NULL;
	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		cout << "getaddrinfo failed with error: " << iResult << endl;
		//printf("getaddrinfo failed with error: %d\n", iResult);
		exit(0);
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket < 0) {
		cout << "socket failed " << endl;
		freeaddrinfo(result);
    exit(0);
	}
  int yes = 1;
  if (setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    perror("setsockopt");
    exit(1);
  }

	// Setup the TCP listening socket
	if ((bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen)) < 0)
  {
		cout << "Bind Failure!\n";
		freeaddrinfo(result);
		close(ListenSocket);
		exit(0);
	}

	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) < 0) 
  {
    cout << "Listen Failure!\n";
		close(ListenSocket);
		exit(0);
	}
	
*/
