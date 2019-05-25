#include <stdio.h>
#include <stdlib.h>

#include "myclib.h"
#include "linkedlist.h"


typedef struct ListEntry_ ListEntry;

struct ListEntry_ {
    void* entry;
    ListEntry* next;
    ListEntry* prev;
};

struct LinkedList_ {
    ListEntry* head;
    ListEntry* tail;
    ListEntry* ptr;
    unsigned int size;
};

LinkedList* LinkedList_New(void) {
    LinkedList* self = (LinkedList*)my_malloc(sizeof(LinkedList));
    self->ptr = NULL;
    self->head = NULL;
    self->tail = NULL;
    self->size = 0;
    return self;
}


void* LinkedList_Next(LinkedList* self) {
    if (self->ptr == NULL) {
        self->ptr = self->head;
    } else {
        self->ptr = self->ptr->next;
    }

    if (self->ptr == NULL) {
        return NULL;
    }
    return self->ptr->entry;
}

void LinkedList_Remove(LinkedList *self, void *entry) {
    ListEntry *target = NULL;
    if (self->ptr != NULL && self->ptr->entry == entry) {
        target = self->ptr;
    } else {
        target = self->head;
        while (target != NULL && target->entry != entry) {
            target = target->next;
        }
    }
    if (target == NULL) {
        // not found
        return;
    }

    if (target == self->head && self->size == 1) {
        // head == prev == target
        self->head = NULL;
        self->tail = NULL;
        self->ptr = NULL;
    } else if (target == self->head) {
        // head == target and there are 2 or more items.
        self->head = target->next;
        self->head->prev = NULL;
    } else if (target == self->tail) {
        // head == tail and there are 2 or more items
        self->tail = target->prev;
        self->tail->next = NULL;
    } else {
        target->prev->next = target->next;
        target->next->prev = target->prev;
    }
    self->size--;
    self->ptr = NULL;
    my_free(target);
}

void* LinkedList_Prev(LinkedList* self) {
    if (self->ptr == NULL) {
        self->ptr = self->tail;
    } else {
        self->ptr = self->ptr->prev;
    }

    if (self->ptr == NULL) {
        return NULL;
    }
    return self->ptr->entry;
}

int LinkedList_Add(LinkedList* self, void* entry) {
    if (self->head == NULL) {
        self->tail = self->head = (ListEntry*)my_calloc(1, sizeof(ListEntry));
        if (!self->tail) {
            return -1;
        }
    } else {
        ListEntry* tmp = self->tail;
        self->tail = (ListEntry*)my_calloc(1, sizeof(ListEntry));
        if (!self->tail) {
            return -1;
        }
        tmp->next = self->tail;
        self->tail->prev = tmp;
    }
    self->size++;
    self->tail->entry = entry;
    return 0;
}


void LinkedList_Delete(LinkedList* self) {
    ListEntry* entry = self->head;
    while(entry != NULL) {
        ListEntry* next = entry->next;
        my_free(entry);
        entry = next;
    }
    my_free(self);
}


unsigned int LinkedList_Size(LinkedList *self) {
    return self->size;
}
