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
    _DIC_ERRORID_NONE = 0x600000000,
    _DIC_ERRORID_CREATEDIC_MALLOC = 0x600010200,
    _DIC_ERRORID_CREATEDIC_HASH = 0x600010201,
    _DIC_ERRORID_CREATEDIC_MALLOCLIST = 0x600010202
};

#define _DIC_ERRORMES_MALLOC "Unable to allocate memory (Size: %lu)"
#define _DIC_ERRORMES_CREATEHASH "Unable to create hash"

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

// Creates a empty dictionary
// Size: The size of the dict list, this should be about the same size as the expected number of entries
DIC_Dict *DIC_CreateDict(size_t Size);

void DIC_InitStructLinkList(DIC_LinkList *Struct);
void DIC_InitStructDict(DIC_Dict *Struct);

void DIC_DestroyLinkList(DIC_LinkList *LinkList);
void DIC_DestroyDict(DIC_Dict *Dict);

DIC_Dict *DIC_CreateDict(size_t Size)
{
    // Allocate memory
    DIC_Dict *Dict = (DIC_Dict *)malloc(sizeof(DIC_Dict));

    if (Dict == NULL)
    {
        _DIC_AddErrorForeign(_DIC_ERRORID_CREATEDIC_MALLOC, strerror(errno), _DIC_ERRORMES_MALLOC, sizeof(DIC_Dict));
        return NULL;
    }

    // Initialize
    DIC_InitStructDict(Dict);

    // Create hash
    Dict->hash = HAS_CreateHash(1, 0);

    if (Dict->hash == NULL)
    {
        _DIC_AddErrorForeign(_DIC_ERRORID_CREATEDIC_HASH, HAS_GetError(), _DIC_ERRORMES_CREATEHASH);
        DIC_DestroyDict(Dict);
        return NULL;
    }

    // Get memory for the list
    Dict->length = Size;
    Dict->list = (DIC_LinkList **)malloc(sizeof(DIC_LinkList *) * Size);

    if (Dict->list == NULL)
    {
        _DIC_AddErrorForeign(_DIC_ERRORID_CREATEDIC_MALLOCLIST, strerror(errno), _DIC_ERRORMES_MALLOC, sizeof(DIC_LinkList *) * Size);
        DIC_DestroyDict(Dict);
        return NULL;
    }

    // Initialize list
    for (DIC_LinkList **List = Dict->list, **EndList = Dict->list + Size; List < EndList; ++List)
        *List = NULL;

    return Dict;
}

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

    free(LinkList);
}

void DIC_DestroyDict(DIC_Dict *Dict)
{
    if (Dict->hash != NULL)
        HAS_DestroyHash(Dict->hash);

    if (Dict->list != NULL)
    {
        for (DIC_LinkList **List = Dict->list, **EndList = Dict->list + Dict->length; List < EndList; ++List)
            if (*List != NULL)
                DIC_DestroyLinkList(*List);

        free(Dict->list);
    }

    free(Dict);
}

#endif