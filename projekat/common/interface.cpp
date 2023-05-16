#include "pch.h"
#include "framework.h"
#include "interface.h"
#include "sock_functions.h"
#include <stdlib.h>
#include <stdio.h>

void Connect(SOCKET connectSocket, Message queueName) {

	// koristi se u select funkciji kako bi se pratilo vise socket descriptora odjednom
	// serveru se salje komanda za konektovanjem na grupu
	// server proverava da li grupa postoji u hash tabeli i na osnovu toga dodaje novu ili samo na 
	// postojecu doda acceptedSocket
	// server ce raspakovati strukturu Message, videti sta je u polju command i na osnovu toga dalje raditi

	FD_SET set;
	timeval timeVal;
	FD_ZERO(&set);
	FD_SET(connectSocket, &set);
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	int iResult = select(0, NULL, &set, NULL, &timeVal);
	if (iResult == SOCKET_ERROR)
	{
		printf("Error in select: %ld\n", WSAGetLastError());
	}
	else if (iResult == 0)
	{
		Sleep(1000);
	}
	else if (iResult > 0)
	{
		iResult = send(connectSocket, (char*)&queueName, (int)sizeof(Message), 0);
	}

	if (iResult == SOCKET_ERROR)
	{
		printf("Send failed with error: %ld\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
	}
}

void SendMess(SOCKET connectSocket, Message message) {
	// serveru se salje komanda za slanjem poruka i poruka koja se zeli poslati 
	FD_SET set;
	timeval timeVal;
	FD_ZERO(&set);
	FD_SET(connectSocket, &set);
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	int iResult = select(0, NULL, &set, NULL, &timeVal);
	if (iResult == SOCKET_ERROR)
	{
		printf("Error in select: %ld\n", WSAGetLastError());
	}
	else if (iResult == 0)
	{
		Sleep(1000);
	}
	else if (iResult > 0)
	{
		iResult = send(connectSocket, (char*)&message, (int)sizeof(Message), 0);
	}

	if (iResult == SOCKET_ERROR)
	{
		printf("Send failed with error: %ld\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
	}
}

void Disconnect(SOCKET connectSocket, Message disconnectMe) {
	// saljemo samo komandu za diskonektovanjem, polje message bice prazno

	FD_SET set;
	timeval timeVal;
	FD_ZERO(&set);
	FD_SET(connectSocket, &set);
	timeVal.tv_sec = 0;
	timeVal.tv_usec = 0;

	int iResult = select(0, NULL, &set, NULL, &timeVal);
	if (iResult == SOCKET_ERROR)
	{
		printf("Error in select: %ld\n", WSAGetLastError());
	}
	else if (iResult == 0)
	{
		Sleep(1000);
	}
	else if (iResult > 0)
	{
		iResult = send(connectSocket, (char*)&disconnectMe, (int)sizeof(Message), 0);
		if (iResult == SOCKET_ERROR)
		{
			printf("Send failed with error: %ld\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
		}
	}
}