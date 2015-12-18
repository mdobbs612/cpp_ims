#define WINVER 0x0501

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// compile with g++ myclient.cpp -l ws2_32 -o serv (or whatever name)
// the -l ws2_32 is the important part

#define BUFFER_LEN 512
#define PORT_NO "27011"

int main(int argc, char **argv)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	SOCKET ConnectSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL, *res_ptr = NULL, hints;

	const char *sendbuffer = "This is my test string";
	char recvbuf[BUFFER_LEN];
    int recvbuflen = BUFFER_LEN;
    int recvResult;
    int sendResult;

    // check if the input was done correctly
    if (argc != 2) {
    	fprintf(stderr, "usage: call client.exe with the server-name as the first parameter\n");
    	return 1;
    }

    // initializing winsock

	wVersionRequested = MAKEWORD( 2, 2 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		fprintf(stderr, "WSAStartup failure. Error: %d\n", err);
        return 1;
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// server address and port
	err = getaddrinfo(NULL, PORT_NO, &hints, &result);
	if ( err != 0) {
		fprintf(stderr, "getaddrinfo failure. Error: %d\n", err);
		WSACleanup();
		return 1;
	}

	// try connecting to addr in result until one succeeds
	for(res_ptr = result; res_ptr != NULL; res_ptr = res_ptr->ai_next) {

		ConnectSocket = socket(res_ptr->ai_family, res_ptr->ai_socktype, 
			res_ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			fprintf(stderr, "socket failure. Error: %d\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		err = connect( ConnectSocket, res_ptr->ai_addr, 
			(int)res_ptr->ai_addrlen);
		if (err == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if(ConnectSocket == INVALID_SOCKET) {
		fprintf(stderr, "Error connecting to server\n");
		WSACleanup();
		return 1;
	}

	// Sending a buffer
	sendResult = send(ConnectSocket, sendbuffer, (int)strlen(sendbuffer), 0);
	if (sendResult == SOCKET_ERROR) {
		fprintf(stderr, "send failure. Error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	printf("Bytes Sent: %d\n", sendResult);

	//  shutting down the connection
	err = shutdown(ConnectSocket, SD_SEND);
	if (err == SOCKET_ERROR) {
		fprintf(stderr, "shutdown failure. Error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}


	// receive until the server closes the connection
	do {

		recvResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if ( recvResult > 0)
			printf("Bytes Received: %d\n", recvResult);
		else if (recvResult == 0)
			printf("Connection has been closed\n");
		else 
			fprintf(stderr, "recv failure. Error: %d\n", WSAGetLastError());
	} while (recvResult > 0);

	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}