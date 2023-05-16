#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include "interface.h"

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27016

bool InitializeWSA();
bool ConnectSocket(SOCKET* connectSocket);

