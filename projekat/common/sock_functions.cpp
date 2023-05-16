#include "pch.h"
#include "framework.h"
#include "sock_functions.h"
#include <stdio.h>
#include <stdlib.h>

bool InitializeWSA() {
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

bool ConnectSocket(SOCKET* connectSocket) {

	// create a socket
	*connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);
	if (*connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return false;
	}

	// Create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;								// IPv4 protocol
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
	serverAddress.sin_port = htons(SERVER_PORT);					// server port

	// Connect to server specified in serverAddress and socket connectSocket
	if (connect(*connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(*connectSocket);
		WSACleanup();
		return false;
	}

	unsigned long mode = 1; //non-blocking mode
	int iResult = ioctlsocket(*connectSocket, FIONBIO, &mode);
	if (iResult != NO_ERROR) {

		printf("ioctlsocket failed with error: %ld\n", iResult);
		return false;
	}

	return true;
}