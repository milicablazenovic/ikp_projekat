#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <WinSock2.h>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "conio.h"

#include "interface.h"

#define QUEUE_EMPTY "RED JE PRAZAN"
#define MAX_MESSAGE_SIZE 256


#define MAX_GROUP_NAME 256
#define HASH_TABLE_SIZE 10
#define MAX_SOCKETS_IN_GROUP 10

#pragma region list_structs
typedef struct list_socket_item {
	SOCKET socket;
	struct list_socket_item* next;
} list_socket_item;
//zna se lista konkretna sa nekim properijima kao 
typedef struct list_socket {
	list_socket_item* head;
	int len;
	int limit;
} list_socket;
#pragma endregion list_structs

#pragma region queue_structs
typedef struct node {
	char* message;
	struct node* next;
} node;

typedef struct queue {
	CRITICAL_SECTION cs;
	node* head;
	node* tail;
	//int queue_counter;
} queue;
#pragma endregion queue_structs

#pragma region hash_structs
typedef struct hashtable_item {
	char* group_name;
	list_socket* sockets;
	queue* group_queue;
	struct hashtable_item* next;
} hashtable_item;
//nasa tabela....
typedef struct {
	CRITICAL_SECTION cs;
	hashtable_item* items;
} hash_table;
#pragma endregion hash_structs


#pragma region list_declaration
// inicijalizuje se nova lista soketa
// vraca pok na tu listu ili null
list_socket* init_list();
// dodavanje soketa na kraj liste|true ako uspe, false ako ne
bool list_add(list_socket* list, SOCKET sock);
// uklanja bilo koji soket NADAM SE DA RADI THIS IS THE BEST I CAN
bool list_remove(list_socket* list, SOCKET sock);
bool list_dump(list_socket* list);
#pragma endregion list_declaration

#pragma region hash_declaration
unsigned int hash(char* group_name);
hash_table* init_hash_table();
bool hashtable_addgroup(hash_table* ht, char* group_name);
bool hashtable_addsocket(hash_table* ht, char* group_name, SOCKET new_socket);
bool hashtable_removesocket(hash_table* ht, char* group_name, SOCKET socket);
list_socket* hashtable_getsockets(hash_table* ht, char* group_name);
hashtable_item* hashtable_getgroup(hash_table* ht, char* group_name);
bool hashtable_dump(hash_table* ht);
void hashtable_print(hash_table* ht);
bool hashtable_findgroup(hash_table* ht, char* group_name);
#pragma endregion hash_declaration

#pragma region queue_declaration
void init_queue(queue* q); // samo postavlja head i tail na null
bool enqueue(queue* q, char* message); // dodavanje poruke na kraj reda
char* dequeue(queue* q); // preuzimanje poruke s pocetka reda#pragma once
queue* getqueue(hash_table* ht, char* group_name);
void queue_dump(queue* q);
#pragma endregion queue_declaration

