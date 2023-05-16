#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define SafeCloseHandle(handle) if(handle) CloseHandle(handle);

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include "../common/interface.cpp" 
#include "../common/sock_functions.cpp"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning (disable: 4996)

#define BUFFER_SIZE 256
#define MAX_LEN_GROUP_NAME 256

// thread za prihvatanje poruka sa strane servera
DWORD WINAPI ThreadRECV(LPVOID lpParam);
char groupName[MAX_MESSAGE_LENGTH];
HANDLE hRecv;
int messageLen;

int main()
{
	SOCKET connectSocket = INVALID_SOCKET;
	int loopIsActive = 0;
	int option = 0;

	if (InitializeWSA() == false) {
		return 1;
	}

	if (ConnectSocket(&connectSocket) == false) {
		return 1;
	}

	hRecv = CreateThread(NULL, NULL, &ThreadRECV, &connectSocket, NULL, NULL);

	while (loopIsActive != 1) {
		printf("------------ DOBRODOSLI U SERVIS ZA GRUPNI CET :D--------\n");
		printf("1. PRIKLJUCI SE GRUPI\n");
		printf("2. IZADJI\n");
		printf("-------------------------------------------------------------\n");
		option = _getch(); // ascii vrednost karaktera

		switch (option - 48) {
		case 1:
			printf("Unesite ime grupe u kojoj zelite slati poruke: \n");
			Message queueName;
			gets_s(queueName.message, MAX_MESSAGE_LENGTH);
			strcpy_s(groupName, MAX_MESSAGE_LENGTH, queueName.message);
			queueName.command = '1';

			messageLen = strlen(queueName.message);
			if (messageLen == 0) {
				printf("Uneli ste prazno ime grupe. Pokusajte opet.\n");
				break;
			}

			Connect(connectSocket, queueName);

			// nakon konektovanja otvara se novi meni za mogucnost slanja poruke
			while (loopIsActive != 1) {
				printf("-------------------------------------------------------------\n");
				printf("1. POSALJI PORUKU\n");
				printf("2. IZADJI\n");
				printf("-------------------------------------------------------------\n");

				option = _getch();
				switch (option - 48)
				{
				case 1:
					printf(">>>");
					Message message;
					gets_s(message.message, MAX_MESSAGE_LENGTH);

					messageLen = strlen(message.message);
					if (messageLen == 0) {
						printf("Uneli ste praznu poruku. Pokusajte opet.\n");
						break;
					}

					strcat(message.message, "#");
					strcat(message.message, groupName);

					message.command = '2';

					SendMess(connectSocket, message);

					break;
				case 2:
					// zahtev za diskonekcijom
					loopIsActive = 1;
					Message disconnectMe;
					disconnectMe.command = '0';
					strcpy(disconnectMe.message, groupName);

					Disconnect(connectSocket, disconnectMe);

					break;
				default:
					printf("Broj koji ste uneli nije validan.Pokusajte opet.\n");
					break;
				}
			}

			break;
		case 2:
			// zahtev za diskonekcijom
			loopIsActive = 1;
			Message disconnectMe;
			disconnectMe.command = '0';
			strcpy(disconnectMe.message, groupName);

			Disconnect(connectSocket, disconnectMe);

			break;
		default:
			printf("Broj koji ste uneli nije validan.Pokusajte opet.\n");
			break;
		}
		printf("\n\n");
	}

	closesocket(connectSocket);
	CloseHandle(hRecv);
	WSACleanup();
	return 0;

}

DWORD WINAPI ThreadRECV(LPVOID lpParam) {

	int iResult;
	SOCKET connectSocket = *(SOCKET*)lpParam;

	// buffer za smestanje pristiglih podataka od servera
	char dataBuffer[BUFFER_SIZE];

	FD_SET set;
	timeval timeVal;
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	bool serverOut = false;

	while (1) {

		FD_ZERO(&set);
		FD_SET(connectSocket, &set);

		iResult = select(0, &set, NULL, NULL, &timeVal);

		// select error
		if (iResult == SOCKET_ERROR)
		{
			serverOut = true;
			
			printf("select failed: %ld\n", WSAGetLastError());
			printf("SERVER UGASEN\n");
			SafeCloseHandle(hRecv);
			return 0;
		}
		// nista sacekaj
		if (iResult == 0)
		{
			Sleep(1000);
			continue;
		}
		// spreman za recv
		if (iResult > 0) {
			// PRIMI PORUKU OD SERVERA
			FD_SET set;
			timeval timeVal;
			timeVal.tv_sec = 0;
			timeVal.tv_usec = 0;
			FD_ZERO(&set);
			FD_SET(connectSocket, &set);

			iResult = select(0, &set, NULL, NULL, &timeVal);
			// select error u recv funkciji
			if (iResult == SOCKET_ERROR)
			{
				printf("select failed in ReceiveFunction: %ld\n", WSAGetLastError());
			}
			// nista sacekaj
			else if (iResult == 0)
			{
				Sleep(1000);
				continue;
			}
			else if (iResult > 0)
			{
				iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);

				if (iResult > 0) {
					// recv ok prosao, ispisi poruku
					printf(">>: %s\n", dataBuffer);
				}
				else {
					// there was an error during recv
					printf("recv failed with error: %d\n", WSAGetLastError());
					closesocket(connectSocket);
				}
			}
		}
	}

	return 0;
}