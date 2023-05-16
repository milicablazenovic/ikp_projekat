#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define SafeCloseHandle(handle) if(handle) CloseHandle(handle);

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include "../common/structures.cpp"


#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_PORT 27016
#define BUFFER_SIZE 256
#define MAX_CLIENTS 100
#define MAX_MESSAGE_SIZE 256

DWORD WINAPI clientHandle(LPVOID params);
DWORD WINAPI sendMessagesToAll(LPVOID lpParam);
HANDLE hSendMess;
HANDLE hRecvMess[MAX_CLIENTS];

hash_table* ht = NULL;

typedef struct pom {
	SOCKET my_socket;
	int idx;
} pom;


int main()
{
	// Socket used for listening for new clients 
	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET acceptedSocket;
	short cltNum = 0;
	//SOCKET acceptedSocket = INVALID_SOCKET;
	//memset(hRecvMess, 0, MAX_CLIENTS);

	// inicijalizacija hash tabele
	ht = init_hash_table();
	// Variable used to store function return value
	int iResult;

	// Buffer used for storing incoming data
	char dataBuffer[BUFFER_SIZE];

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}


	// Initialize serverAddress structure used by bind
	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;				// IPv4 address family
	serverAddress.sin_addr.s_addr = INADDR_ANY;		// Use all available addresses
	serverAddress.sin_port = htons(SERVER_PORT);	// Use specific port

	//memset(acceptedSockets, 0, MAX_CLIENTS * sizeof(SOCKET));
	// Create a SOCKET for connecting to server
	listenSocket = socket(AF_INET,      // IPv4 address family
		SOCK_STREAM,  // Stream socket
		IPPROTO_TCP); // TCP protocol

    // Check if socket is successfully created
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket - bind port number and local address to socket
	iResult = bind(listenSocket, (struct sockaddr*) & serverAddress, sizeof(serverAddress));

	// Check if socket is successfully binded to address and port from sockaddr_in structure
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	bool bOptVal = true;
	int bOptLen = sizeof(bool);
	iResult = setsockopt(listenSocket, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char*)&bOptVal, bOptLen);
	if (iResult == SOCKET_ERROR) {
		printf("setsockopt for SO_CONDITIONAL_ACCEPT failed with error: %u\n", WSAGetLastError());
	}

	// Set listenSocket in listening mode
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Server socket is set to listening mode. Waiting for new connection requests.\n");

	FD_SET set;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;
	do {
		FD_ZERO(&set);
		FD_SET(listenSocket, &set);

		int selectResult = select(0, &set, NULL, NULL, &timeVal);

		if (selectResult == SOCKET_ERROR)
		{
			printf("Select failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
		else if (selectResult == 0) // timeout expired
		{
			
			continue;
		}
		else if (FD_ISSET(listenSocket, &set))
		{
			sockaddr_in clientAddr;
			int clientAddrSize = sizeof(struct sockaddr_in);
			acceptedSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
			if (acceptedSocket == INVALID_SOCKET)
			{
				printf("accept failed with error: %d\n", WSAGetLastError());
				
			}
			else {
				unsigned long int nonBlockingMode = 1;
				if (ioctlsocket(acceptedSocket, FIONBIO, &nonBlockingMode))
				{
					printf("ioctlsocket failed with error.");
					continue;
				}
				
				printf("New client request accepted. Client address: %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
				pom params = {acceptedSocket,cltNum };
				hRecvMess[cltNum] = CreateThread(NULL, NULL, &clientHandle, &params, NULL, NULL);
				cltNum++;
			}
		}
	} while (!_kbhit());

	//Close listen and accepted sockets
	//_getch();
	closesocket(listenSocket);
	
	//close thread's handle
	for (int i = 0; i < cltNum; i++)
		CloseHandle(hRecvMess[i]);
	CloseHandle(hSendMess);
	hashtable_dump(ht);
	// Deinitialize WSA library
	WSACleanup();
	_getch();
	return 0;
}

// thread za prihvatanje konekcija, upisivanje grupa i soketa u hash tabelu, stavljanje poruka u red

DWORD WINAPI clientHandle(LPVOID params)
{
	pom p = *(pom*)params;
	SOCKET acceptedSocket = p.my_socket;// koji klijent se konektovao
	int ind = p.idx;
	printf("Nova konekcija prihvacena.Soket: %d\n", acceptedSocket);

	FD_SET set;
	timeval timeVal;
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;
	bool odjavljen = false;
	int iResult;

	do{
		FD_ZERO(&set);
		FD_SET(acceptedSocket, &set);

		char dataBuffer[BUFFER_SIZE]; // bafer za smestanje poruka
		if (odjavljen)
			break;
		iResult = select(0, &set, NULL, NULL, &timeVal);
		if (iResult == SOCKET_ERROR)
		{
			printf("select failed: %ld\n", WSAGetLastError());
			closesocket(acceptedSocket);
			odjavljen = true;
		}
		else if (iResult == 0)
		{
			Sleep(1000);
			continue;
		}
		else
		{
			int iResult = recv(acceptedSocket, dataBuffer, BUFFER_SIZE, 0);

			if (iResult > 0)	// Check if message is successfully received
			{
				Message* message = (Message*)dataBuffer;

				switch (message->command) {
				case '0':
				
					if (hashtable_removesocket(ht, (message->message), acceptedSocket)) {
						printf("Klijent %d se diskonektovao!\n", acceptedSocket);
						list_socket* lista = hashtable_getsockets(ht, (message->message));
						list_print(lista->head, (message->message));
					}
					odjavljen = true;
					closesocket(acceptedSocket);
					SafeCloseHandle(hRecvMess[ind]);
					break;
				case '1':
				{
					printf("Poruka od klijenta (grupa na koju se zeli konektovati): %s.\n", message->message);
					printf("_______________________________  \n");

					//dodavanje grupe u hash tabelu i acceptedSocketa u listu soketa te grupe
					//odgovor klijentu da je povezan na grupu koju je trazio

					if (!hashtable_findgroup(ht, (message->message))) {
						hashtable_addgroup(ht, (message->message));

						hashtable_addsocket(ht, (message->message), acceptedSocket);
					}
					else {
						hashtable_addsocket(ht, (message->message), acceptedSocket);
					}
					char messageToSend[BUFFER_SIZE];
					strcpy_s(messageToSend, "povezani ste na trazenu grupu i spremni za slanje poruka\n");

					iResult = send(acceptedSocket, messageToSend, MAX_MESSAGE_SIZE, 0);

					if (iResult == SOCKET_ERROR)
					{
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(acceptedSocket);
						return 1;
					}

					hSendMess = CreateThread(NULL, NULL, &sendMessagesToAll, message->message, NULL, NULL);
				}
					break;
				case '2':
				{
					// dodati poruku u red

					char delimiter[] = "#";
					char* message_from_client, * client_group;
					message_from_client = strtok(message->message, delimiter);
					client_group = strtok(NULL, delimiter);

					printf("Poruka od klijenta: %s.\n", message_from_client);
					printf("_______________________________  \n");

					
					char messageToSend[BUFFER_SIZE];
					strcpy_s(messageToSend, "Uspesno ste poslali poruku u grupni chat!\n");
					iResult = send(acceptedSocket, messageToSend, MAX_MESSAGE_SIZE, 0);

					if (iResult == SOCKET_ERROR)
					{
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(acceptedSocket);
						return 1;
					}

					enqueue(getqueue(ht, (client_group)), (message_from_client));
				}
					break;
				}
			}
			else if (iResult == 0)	// Check if shutdown command is received
			{
				// Connection was closed successfully
				printf("Connection with client closed.\n");
				closesocket(acceptedSocket);
			}
			else	// There was an error during recv
			{

				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(acceptedSocket);
			}
		}
	}while(true);
	printf("Klijent se zatvorio!\n");
	return 0;
}

// thread za proveravanje da li ima poruka u redu i ako ima poslati svima
// a ako nema sleep 5 sek

DWORD WINAPI sendMessagesToAll(LPVOID lpParam) {
	char* group = (char*)lpParam;
	char* pom = (char*)malloc(sizeof(char)* MAX_MESSAGE_LENGTH);
	queue* kju = getqueue(ht, group);
	list_socket* lista = hashtable_getsockets(ht, group);
	int len = lista->len;
	list_socket_item* soketi = lista->head;

	printf("Duzina liste soketa: %d\n", len);
	list_print(lista->head, group);

	do {
		if (kju->head != NULL) {
			pom = dequeue(kju);
			while (soketi != NULL) {
				if (soketi->next == NULL) {
					int iResult = send(soketi->socket, pom, MAX_MESSAGE_SIZE, 0);

					if (iResult == SOCKET_ERROR)
					{
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(soketi->socket);
						return 1;
					}
				}
				else {					
					int iResult = send(soketi->socket, pom, MAX_MESSAGE_SIZE, 0);

					if (iResult == SOCKET_ERROR)
					{
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(soketi->socket);
						return 1;
					}
				}
				soketi = soketi->next;
				
			}
			soketi = lista->head;
		}
		else {
			Sleep(5000);
			continue;
		}
	} while (1);
	free(pom);
	return 0;
}