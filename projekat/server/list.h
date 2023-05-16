#pragma once
#include <minwinbase.h>
#include <WinSock2.h>

#define MAX_GROUP_NAME 256

typedef struct Sockets {
	SOCKET socket;
	Sockets* next;
}Sockets;

typedef struct Group {
	char group_name[MAX_GROUP_NAME];
	queue* group_queue;
	Sockets* group_sockets;
}Group;

typedef struct Groups {
	Group* group;
	Groups* next;
}Groups;

typedef struct node {
	char* message;
	struct node* next;
} node;

typedef struct queue {
	CRITICAL_SECTION cs;
	node* head;
	node* tail;
	//int queue_counter;
}queue;