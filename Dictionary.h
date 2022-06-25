#ifndef DICTIONARY_H_INCLUDED
#define DICTIONARY_H_INCLUDED

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <Hashing.h>

#define ERR_PREFIX DIC
#include <Error.h>

enum _DIC_ErrorID {
    _DIC_ERRORID_NONE = 0x600000000
};

typedef struct __DIC_Dict DIC_Dict;
typedef struct __DIC_LinkList DIC_LinkList;

struct __DIC_LinkList {
    uint8_t *key;
    size_t keyLength;
    void *value;
    bool pointer;
    DIC_LinkList *next;
};

struct __DIC_Dict {
    HAS_Hash *hash;
    DIC_LinkList **list;
    size_t length;
};

void DIC_InitStructLinkList(DIC_LinkList *Struct);
void DIC_InitStructDict(DIC_Dict *Struct);

void DIC_DestroyLinkList(DIC_LinkList *LinkList);
void DIC_DestroyDict(DIC_Dict *Dict);

void DIC_InitStructLinkList(DIC_LinkList *Struct)
{
    Struct->key = NULL;
    Struct->keyLength = 0;
    Struct->value = NULL;
    Struct->pointer = false;
    Struct->next = NULL;
}

void DIC_InitStructDict(DIC_Dict *Struct)
{
    Struct->hash = NULL;
    Struct->list = NULL;
    Struct->length = 0;
}

void DIC_DestroyLinkList(DIC_LinkList *LinkList)
{
    // Destroy the key
    if (LinkList->key != NULL)
        free(LinkList->key);

    if (LinkList->pointer && LinkList->value != NULL)
        free(LinkList->value);

    if (LinkList->next != NULL)
        DIC_DestroyLinkList(LinkList->next);
}

void DIC_DestroyDict(DIC_Dict *Dict)
{

}

#endif