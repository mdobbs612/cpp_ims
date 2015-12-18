#define WINVER 0x0501

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// compile with g++ myserver.cpp -l ws2_32 -o serv (or whatever name)
// the -l ws2_32 is the important part

#define BUFFER_LEN 512
#define PORT_NO "27011"

int main(int argc, char **argv)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    char recvbuf[BUFFER_LEN];
    int recvbuflen = BUFFER_LEN;
    int recvResult;
    int sendResult;

    // initializing winsock

	wVersionRequested = MAKEWORD( 2, 2 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		fprintf(stderr, "WSAStartup failure. Error: %d\n", err);
        return 1;
	}

	//ZeroMemory(&hints, sizeof(hints));
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// server address and port
	err = getaddrinfo(NULL, PORT_NO, &hints, &result);
	if ( err != 0) {
		fprintf(stderr, "getaddrinfo failure. Error: %d\n", err);
		WSACleanup();
		return 1;
	}

	// socket for connecting to the server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		fprintf(stderr, "socket failure. Error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// set it up for listening
	err = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (err == SOCKET_ERROR) {
		fprintf(stderr, "bind failure. Error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	err = listen(ListenSocket, SOMAXCONN);
	if (err == SOCKET_ERROR) {
		fprintf(stderr, "listen failure. Error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// accepting client
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		fprintf(stderr, "accept failure. Error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	closesocket(ListenSocket);

	// recieving data until the client closes the connection
	do {

		recvResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (recvResult > 0) {
			printf("Bytes received: %d\n", recvResult);
			printf("%s\n", recvbuf);

			sendResult = send(ClientSocket, recvbuf, recvResult, 0);
			if (sendResult == SOCKET_ERROR) {
				fprintf(stderr, "send failure. Error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			printf("Bytes sent: %d\n", sendResult);
		}
		else if (recvResult == 0)
			printf("Connection closing...\n");
		else {
			fprintf(stderr, "recv failure. Error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

	} while (recvResult > 0);

	// shut down the connection once the client is closed
	err = shutdown(ClientSocket, SD_SEND);
	if (err == SOCKET_ERROR) {
		fprintf(stderr, "shutdown failure. Error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}