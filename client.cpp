#include "client.h"



#define PORT_NO "27012"


void * ListenHandler(void * s);

int main(int argc, char **argv)
{
	printf("Client\n");
	int err;

	int ConnectSocket = -1;
  pthread_t listenThread;

	char sendBuf[BUFFER_LEN];
	//char recvBuf[BUFFER_LEN];
	if (argc != 2) {
		fprintf(stderr, "usage: call client.exe with the server-name as the first parameter\n");
		return 1;
	}

	// initializing winsock



  struct sockaddr_in addr;
  
  if((ConnectSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    cout << "Error creating socket\n";
    return 1;
  }

  memset(&addr, '0', sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(27012);

  if(inet_pton(addr.sin_family, argv[1], &(addr.sin_addr)) <= 0)
  {
    cout << "Error inet_pton\n";
    return 1;
  }

  if(connect(ConnectSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    cout << "Connect failure\n";
    return 1;
  }

  int * cs = &ConnectSocket;

  pthread_create(&listenThread, NULL, ListenHandler, (void *)cs);


	while (1) {
		memset(sendBuf, 0, BUFFER_LEN);
		//memset(recvBuf, 0, BUFFER_LEN);
		//fgets(sendBuf, BUFFER_LEN, stdin);
    char line[200];
    cin.getline (line,200);
    char *im;
    char *cmd;  
    char *arg;
    
    cmd = strtok(line, " \n");
    //if (strcmp(cmd, "logout" == 0) {
      arg = strtok(NULL, " \n");
   // }
    if (strcmp(cmd, "im") == 0) {
      im = strtok(NULL, " \n");
      //cout << cmd << arg << im << endl;
    }

    if (strcmp(cmd, "exit") == 0) {
      break;
    }

    //cout << cmd << arg << endl;
		//cin >> cmd >> arg;
		//memcpy(sendBuf, ParseInput(cmd, arg), strlen(sendBuf));
    //cout << "SENDING " << sendBuf << endl;
    cout << "PARSA " << ParseInput(cmd, arg) << endl;
		cout << "SEND: " << send(ConnectSocket, ParseInput(cmd, arg), 244, 0) << endl;
		
    //cout << "RECEIVED: " << recvBuf << endl;
		//ParseString(recvBuf);
		//printf("%s", recvBuf);
	}

	close(ConnectSocket);


	return 0;
}


  


const char*ParseInput(const char*cmd, const char* arg) {
  //cout << "> Parsing Input\n";
	if (!(strcmp("register", cmd))) {
    //cout << "Hi, in here with: " << arg << endl;
		Action_Msg *msg;
		msg = new Action_Msg(REGISTER, "", arg, "");
    //cout << "SERLS " << msg->serialize() << endl;
		return msg->serialize();
	}
	if (!(strcmp("login", cmd))) {
		Action_Msg *msg;
		msg = new Action_Msg(LOGIN, "", arg, "");
		return msg->serialize();
	}
	if (!(strcmp("logout", cmd))) {
		Action_Msg *msg;
		msg = new Action_Msg(LOGOUT, "", "", "");
		return msg->serialize();
	}
	if (!(strcmp("add", cmd))) {
		Action_Msg *msg;
		msg = new Action_Msg(ADD_FRIEND, "", arg, "");
		return msg->serialize();
	}
	if (!(strcmp("rem", cmd))) {
		Action_Msg *msg;
		msg = new Action_Msg(REM_FRIEND, "", arg, "");
		return msg->serialize();
	}
	if (!(strcmp("status", cmd))) {
		Action_Msg *msg;
		if (!(strcmp("all", arg))) {
			msg = new Action_Msg(STATUS_ALL, "", "", "");
		}
		else msg = new Action_Msg(STATUS_ONE, "", arg, "");
		return msg->serialize();
	}
	if (!(strcmp("msg", cmd))) {
		int i = 0;
		char im[200];
		do {
			if (i) cout << "ERROR: Message must be less than 200 characters\n";
			cout << "Enter the message you'd like to send\n";
			cin >> im;
		} while (strlen(im) <= 200);

		Action_Msg *msg;
		msg = new Action_Msg(LOGIN, "", arg, im);
		return msg->serialize();
	}

	return "OOps\n";
} 


void * ListenHandler(void * s)
{
  int ConnectSocket = *((int *) s);
  char recvBuf[BUFFER_LEN];


  while(1) {
    memset(recvBuf, 0, BUFFER_LEN);
    recv(ConnectSocket, recvBuf, BUFFER_LEN, 0);
    ParseString(recvBuf);
  }
}


/*
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	// server address and port
	err = getaddrinfo(NULL, PORT_NO, &hints, &result);

	if (err != 0) {
		fprintf(stderr, "getaddrinfo failure. Error: %d\n", err);
		return 1;
	}


	// try connecting to addr in result until one succeeds
	for (res_ptr = result; res_ptr != NULL; res_ptr = res_ptr->ai_next) {


		ConnectSocket = socket(res_ptr->ai_family, res_ptr->ai_socktype,
			res_ptr->ai_protocol);

		if (ConnectSocket < 0) {
      cout << "Socket failure!\n";
			return 1;

		}

		err = connect(ConnectSocket, res_ptr->ai_addr,
			(int)res_ptr->ai_addrlen);

		if (err != 0) {
			close(ConnectSocket);
			ConnectSocket = -1;

      cout << "Connect errrror\n" << err << endl;
			continue;
		}
		else break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == -1) {
		cout << "Error connecting to server\n";
		return 1;
	}
	*/
