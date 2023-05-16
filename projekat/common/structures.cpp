#include "framework.h"
#include "pch.h"
#include "structures.h"

#pragma region list_definitions
list_socket* init_list() {
	list_socket* socks = (list_socket*)malloc(sizeof(list_socket));
	socks->head = NULL;
	socks->limit = MAX_SOCKETS_IN_GROUP;
	socks->len = 0;

	return socks;
}

bool list_add(list_socket* list, SOCKET sock) {
	list_socket_item* temp_ = list->head;
	//dodavanje samo ako nije popunjena lista
	if (list->len < list->limit) {
		//zauzmemo mem za novi soket
		list_socket_item* new_item = (list_socket_item*)malloc(sizeof(list_socket_item));
		new_item->socket = sock;
		new_item->next = NULL;

		list_socket_item* current = list->head;
		// ako head liste pokazuje na null znaci da je mesto slobodno
		// za dodavanje novog soketa
		if (current == NULL) {
			current = new_item;
			list->head = current;
			list->len++;
			return true;
		}
		else {
			while (current->next != NULL) {
				current = current->next;
			}
			current->next = new_item;
			//list->head = current;
			list->len++;
			//return true; // ?????
		}
		list->head = temp_;
	}
	else {
		printf("WARINING: List is full.\n");
		return false;
	}
	return true;
}
//list_remove(item->sockets, socket);
bool list_remove(list_socket* list, SOCKET sock) {
	list_socket_item* prev = list->head;
	list_socket_item* temp = prev->next;

	if (prev == NULL) {
		printf("WARINING: List is empty.\n");
		return false;
	}
	if (prev->socket == sock)
	{
		list->head = temp;
		free(prev);
		list->len--; // kontam da smanjimo count
		return true;
	}
	while (temp != NULL) {
		if (temp->socket == sock) {
			prev->next = temp->next;
			free(temp);
			list->len--; // kontam treba da smanjimo count za svaki slucaj	
			return true;
		}
		prev = temp;
		temp = temp->next;
	}
	return true;
}

bool list_dump(list_socket* list) {
	if (list == NULL) {
		printf("ERROR: List is NULL\n");
		return false;
	}
	if (list->len == 0) {
		free(list);
		return true;
	}

	list_socket_item* current = list->head;
	list_socket_item* temp;

	while (current != NULL) {
		temp = current->next;
		closesocket(current->socket);
		free(current);
		current = temp;
	}

	list->head = NULL;
	return true;
}

void list_print(list_socket_item* head, char* group) {
	list_socket_item* tmp = head;
	if (head == NULL)
	{
		printf("SOCKETS in GROUP: %s\n", group);
		return;
	}printf("SOCKETS in GROUP: %s\n", group);
	while (tmp != NULL) {
		if (tmp->next == NULL) {
			printf("%d\n", tmp->socket);
		}
		else {
			printf("%d\n", tmp->socket);
		}
		tmp = tmp->next;
	}

}
#pragma endregion list_definitions

#pragma region hash_definitions
unsigned int hash(char* group_name) {
	unsigned int hash = 5381;
	int c;
	while (c = *group_name++) {
		hash = ((hash << 5) + hash) + c;
	}
	return hash % HASH_TABLE_SIZE;
}
// inicijalizuje tabelu, vraca pok na novu tabelu
hash_table* init_hash_table() {
	hash_table* ht = (hash_table*)malloc(sizeof(hash_table));
	ht->items = (hashtable_item*)malloc(sizeof(hashtable_item) * HASH_TABLE_SIZE);


	for (int i = 0; i < HASH_TABLE_SIZE; i++) {
		ht->items[i].sockets = NULL;
		ht->items[i].group_name = (char*)malloc(MAX_GROUP_NAME);
		ht->items[i].group_queue = (queue*)malloc(sizeof(queue*));
		ht->items[i].next = NULL;
	}

	InitializeCriticalSection(&ht->cs);

	return ht;
}

bool addgroup_temp(hash_table* ht, hashtable_item* item, char* group_name) {
	if (strcpy_s(item->group_name, MAX_GROUP_NAME, group_name) != 0) {
		printf("ERROR: Adding new group to hash table failed. :(\n");
		//LeaveCriticalSection(&ht->cs);
		return false;
	}
	//dodali smo grupu sad i listu
	item->sockets = init_list();
	if (item->sockets == NULL) {
		printf("ERROR: Adding new list of sockets to the group in hash table failed. :(\n");
		//LeaveCriticalSection(&ht->cs);
		return false;
	}
	//i njen red treba da inicijalizujemo
	init_queue(item->group_queue);

	return true;
}
// dodaje novu grupu i njenu listu soketa
bool hashtable_addgroup(hash_table* ht, char* group_name) {
	int index = hash(group_name);
	bool ret = false;
	EnterCriticalSection(&ht->cs);
	hashtable_item* item = &(ht->items[index]);
	
	
	// lista soketa na grupi je prazna...
	if (item->sockets == NULL) {
		////Zero if successful; otherwise, an error.
		//if (strcpy_s(item->group_name, MAX_GROUP_NAME, group_name) != 0) {
		//	printf("ERROR: Adding new group to hash table failed. :(\n");
		//	LeaveCriticalSection(&ht->cs);
		//	return false;
		//}
		//		//dodali smo grupu sad i listu
		//item->sockets = init_list();
		//if (item->sockets == NULL) {
		//	printf("ERROR: Adding new list of sockets to the group in hash table failed. :(\n");
		//	LeaveCriticalSection(&ht->cs);
		//return false;
		//}
		//		//i njen red treba da inicijalizujemo
		//init_queue(item->group_queue);

		ret = addgroup_temp(ht, item, group_name);
	}
	else {
		printf("WARNING: Collision in hash table.\n");
		while (1) {
			if (item->next == NULL) {
				item->next = (hashtable_item*)malloc(sizeof(hashtable_item));
				item->next->next = NULL;
				item->next->sockets = NULL;
				item->next->group_name = (char*)malloc(MAX_GROUP_NAME);
				item->next->group_queue = (queue*)malloc(sizeof(queue*));

				//if (strcpy_s(item->next->group_name, MAX_GROUP_NAME, group_name) != 0) {
				//	printf("ERROR: Adding new group to hash table failed. :(\n");
				//	LeaveCriticalSection(&ht->cs);
				//	return false;
				//}
				////dodali smo grupu sad i listu
				//item->next->sockets = init_list();
				//if (item->next->sockets == NULL) {
				//	printf("ERROR: Adding new list of sockets to the group in hash table failed. :(\n");
				//	LeaveCriticalSection(&ht->cs);
				//	return false;
				//}
				////i njen red treba da inicijalizujemo
				//init_queue(item->next->group_queue);
				ret = addgroup_temp(ht, item->next, group_name);
				break;
			}
			else {
				item = item->next;
			}
		}
		/*printf("ERROR: RADI TESTIRANJA HASH TABLE ADD GROUP KOD KOLIZIJE. :(\n");
		LeaveCriticalSection(&ht->cs);
		return false;*/
	}
	LeaveCriticalSection(&ht->cs);
	return ret;
}
bool hashtable_findgroup(hash_table* ht, char* group_name) {
	int index = hash(group_name);

	EnterCriticalSection(&ht->cs);
	hashtable_item* item = &(ht->items[index]);
	if (item->sockets == NULL) {
		LeaveCriticalSection(&ht->cs);
		return false;
	}	
	LeaveCriticalSection(&ht->cs);
	return true;
}
// dodaje soket na kraj liste tj poslednje slobodno mesto
// update za koliziju
bool hashtable_addsocket(hash_table* ht, char* group_name, SOCKET new_socket) {
	int index = hash(group_name);

	EnterCriticalSection(&ht->cs);
	// moze i da bude hashtable_grupa... ne znam kako da nazovem da bude
	// INTUITIVNO...
	hashtable_item* item = &(ht->items[index]);
	if (item->sockets == NULL) {
		printf("ERROR: Group doesn't exist in hash table.\n");
		LeaveCriticalSection(&ht->cs);
		return false;
	}

	bool ret = list_add(item->sockets, new_socket);

	LeaveCriticalSection(&ht->cs);
	return ret;
}

// update za koliziju
bool hashtable_removesocket(hash_table* ht, char* group_name, SOCKET socket) {
	int index = hash(group_name);

	EnterCriticalSection(&ht->cs);
	hashtable_item* item = &(ht->items[index]);
	if (item->sockets == NULL) {
		printf("ERROR: Group doesn't exist in hash table.\n");
		LeaveCriticalSection(&ht->cs);
		return false;
	}
	bool ret = list_remove(item->sockets, socket);
	LeaveCriticalSection(&ht->cs);
	return ret;
}
// vraca listu svih soketa konektivanih na grupu|NULL kod errora
list_socket* hashtable_getsockets(hash_table* ht, char* group_name) {
	int index = hash(group_name);

	EnterCriticalSection(&ht->cs);
	hashtable_item* item = &(ht->items[index]);
	if (item == NULL) {
		printf("ERROR: Group doesn't exist in hash table.\n");
		LeaveCriticalSection(&ht->cs);
		return NULL;
	}
	while (item->next != NULL && strncmp(item->next->group_name, group_name, MAX_GROUP_NAME) != 0)
	{
		item = item->next;
	}
	LeaveCriticalSection(&ht->cs);

	return item->sockets;
}

hashtable_item* hashtable_getgroup(hash_table* ht, char* group_name) {
	int index = hash(group_name);

	EnterCriticalSection(&ht->cs);
	hashtable_item* item = &(ht->items[index]);
	if (item == NULL) {
		printf("ERROR: Group doesn't exist in hash table.\n");
		LeaveCriticalSection(&ht->cs);
		return NULL;
	}
	while (item->next != NULL && strncmp(item->next->group_name, group_name, MAX_GROUP_NAME) != 0)
	{
		item = item->next;
	}
	LeaveCriticalSection(&ht->cs);

	return item;
}

queue* getqueue(hash_table* ht, char* group_name) {
	int index = hash(group_name);

	EnterCriticalSection(&ht->cs);
	hashtable_item* item = &(ht->items[index]);
	if (item == NULL) {
		printf("ERROR: Group doesn't exist in hash table.\n");
		LeaveCriticalSection(&ht->cs);
		return NULL;
	}
	while (item->next != NULL && strncmp(item->next->group_name, group_name, MAX_GROUP_NAME) != 0)
	{
		item = item->next;
	}
	LeaveCriticalSection(&ht->cs);

	return item->group_queue;
}
// mora da se update za koliziju
bool hashtable_dump(hash_table* ht) {
	if (ht == NULL) {
		printf("ERROR: Hash table is NULL\n");
		return false;
	}
	EnterCriticalSection(&ht->cs);
	for (int i = 0; i < HASH_TABLE_SIZE; i++) {
		hashtable_item* item = &ht->items[i];

		if (item->sockets == NULL) {
			continue;
		}
		else {
			list_dump(item->sockets);
		}

		if (item->group_name != NULL) {
			free(item->group_name);
		}

		if (item->group_queue != NULL) {
			queue_dump(item->group_queue);
		}

		/*if (item->next != NULL && item->next->sockets != NULL) {
			list_dump(item->sockets);
			free(item->group_queue);
		}*/
	}
	LeaveCriticalSection(&ht->cs);
	free(ht->items);
	DeleteCriticalSection(&ht->cs);
	free(ht);
	return true;
}

void hashtable_print(hash_table* ht) {
	hashtable_item* item;
	list_socket_item* current;

	EnterCriticalSection(&ht->cs);

	for (int i = 0; i < HASH_TABLE_SIZE; i++) {
		item = &(ht->items[i]);

		if (item == NULL || item->sockets == NULL || item->sockets->len == 0) {
			continue;
		}

		current = item->sockets->head;

		printf("\t%i\t", i);
		while (current != NULL) {
			printf("%s - ", current->socket);
			current = current->next;
		}
		printf("\n");
	}

	LeaveCriticalSection(&ht->cs);
}
#pragma endregion hash_definitions

#pragma region queue_definitions
void init_queue(queue* q) {
	q->head = NULL;
	q->tail = NULL;

	InitializeCriticalSection(&(q->cs));
}



bool enqueue(queue* q, char* message) {
	EnterCriticalSection(&(q->cs));
	node* new_node = (node*)malloc(sizeof(node));
	new_node->message = (char*)malloc(sizeof(char) * MAX_MESSAGE_LENGTH);
	new_node->next = NULL;
	if (new_node == NULL) {
		LeaveCriticalSection(&(q->cs));
		return false;
	}
	strcpy_s(new_node->message, MAX_MESSAGE_SIZE, message);
	new_node->next = NULL;

	// ako je tail postojao
	if (q->tail != NULL) {
		q->tail->next = new_node;
	}

	// ako tail nije postojao bice bas newnode
	q->tail = new_node;

	if (q->head == NULL) { // ako nismo imali elemenata tj prosledjeni je prvi koji dodajemo
		q->head = new_node;
	}
	LeaveCriticalSection(&(q->cs));
	return true;
}

char* dequeue(queue* q) {
	EnterCriticalSection(&(q->cs));
	if (q->head == NULL) {
		LeaveCriticalSection(&(q->cs));
		return (char*)QUEUE_EMPTY;
	}
	node* tmp = q->head;
	char* result = q->head->message;
	q->head = q->head->next; // postavimo na sledeci

	if (q->head == NULL) {
		q->tail = NULL; // ako je head null necemo ostaviti tail da visi
	}

	free(tmp); // zapravo skidanje s reda
	LeaveCriticalSection(&(q->cs));
	return result;
}

void queue_dump(queue* q) {
	node* tmp = q->head;
	while (q->head != q->tail) {
		q->head = q->head->next; // postavimo na sledeci

		if (q->head == NULL) {
			q->tail = NULL; // ako je head null necemo ostaviti tail da visi
		}

		free(tmp); // zapravo skidanje s reda
	}

	free(q->tail);
	free(q);
}
#pragma endregion queue_definitions


