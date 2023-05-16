#pragma once
#include <WinSock2.h>

#define MAX_MESSAGE_LENGTH 256

// struktura koja sadrzi komandu koju klijent unese i poruku za slanje
// server ovo dalje obradjuje

typedef struct Message {
	char command;
	char message[MAX_MESSAGE_LENGTH];
}Message;

void Connect(SOCKET connectSocket, Message queueName);
void SendMess(SOCKET connectSocket, Message message);
void Disconnect(SOCKET connectSocket, Message disconnectMe);