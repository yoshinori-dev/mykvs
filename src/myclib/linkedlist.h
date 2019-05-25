#ifndef LINKEDLIST_H
#define LINKEDLIST_H


typedef struct LinkedList_ LinkedList;

LinkedList* LinkedList_New(void);

void* LinkedList_Next(LinkedList *self);

void* LinkedList_Prev(LinkedList *self);

int LinkedList_Add(LinkedList *self, void *entry);

void LinkedList_Delete(LinkedList *self);

unsigned int LinkedList_Size(LinkedList *self);

void LinkedList_Remove(LinkedList *self, void *entry);

#endif /* LINKEDLIST_H */
