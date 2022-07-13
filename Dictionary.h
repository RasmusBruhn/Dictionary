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
    _DIC_ERRORID_CREATEDIC_MALLOCLIST = 0x600010202,
    _DIC_ERRORID_ADDITEM_MALLOCITEM = 0x600020200,
    _DIC_ERRORID_ADDITEM_MALLOCKEY = 0x600020201,
    _DIC_ERRORID_ADDITEM_HASHTABLE = 0x600020202,
    _DIC_ERRORID_ADDITEM_MALLOCVALUE = 0x600020203,
    _DIC_ERRORID_DESTROYDICT_NODICT = 0x600030100,
    _DIC_ERRORID_CHECKITEM_HASHTABLE = 0x600040100,
    _DIC_ERRORID_GETITEM_HASHTABLE = 0x600050200,
    _DIC_ERRORID_GETITEM_NOITEM = 0x600050201,
    _DIC_ERRORID_REMOVEITEM_HASHTABLE = 0x600060200,
    _DIC_ERRORID_REMOVEITEM_NOITEM = 0x600060201
};

#define _DIC_ERRORMES_MALLOC "Unable to allocate memory (Size: %lu)"
#define _DIC_ERRORMES_CREATEHASH "Unable to create hash"
#define _DIC_ERRORMES_WRONGDICTCOUNT "Attempting to destroy dict, but none is supposed to exist"
#define _DIC_ERRORMES_NOHASHTABLE "No hash table is available (Expected number of dicts: %lu)"
#define _DIC_ERRORMES_NOITEM "Unable to locate item"

enum __DIC_Mode {
    DIC_MODE_POINTER,
    DIC_MODE_COPY,
    DIC_MODE_INSERT
};

typedef enum __DIC_Mode DIC_Mode;
typedef enum __DIC_Type DIC_Type;
typedef struct __DIC_Dict DIC_Dict;
typedef struct __DIC_LinkList DIC_LinkList;

struct __DIC_LinkList {
    char *key;
    void *value;
    bool pointer;
    DIC_LinkList *next;
};

struct __DIC_Dict {
    DIC_LinkList **list;
    size_t length;
};

// Creates a empty dictionary
// Size: The size of the dict list, this should be about the same size as the expected number of entries
DIC_Dict *DIC_CreateDict(size_t Size);

// Add an item to a dictionary
// Dict: The dictionary to add the item to
// Key: The key for the item
// KeyLength: The size of the key
// Type: The key type
// Value: A pointer to the value to store
// ValueLength: The size of the value data, only used if copy is true
// Copy: If false then it will save the pointer to the value, if true it will allocate some space and save a copy of the value
bool DIC_AddItem(DIC_Dict *Dict, const char *Key, void *Value, size_t ValueLength, DIC_Mode Mode);

// Remove an item from a dictionary
// Dict: The dictionary to remove an item from
// Key: The key for the item
// KeyLength: The size of the key
// Type: The key type
bool DIC_RemoveItem(DIC_Dict *Dict, const char *Key);

// Get an item from a dictionary
// Dict: The dictionary to remove an item from
// Key: The key for the item
// KeyLength: The size of the key
// Type: The key type
void *DIC_GetItem(DIC_Dict *Dict, const char *Key);

// Checks if an item exists in a dictionary
// Dict: The dictionary to remove an item from
// Key: The key for the item
// KeyLength: The size of the key
// Type: The key type
bool DIC_CheckItem(DIC_Dict *Dict, const char *Key);

void DIC_InitStructLinkList(DIC_LinkList *Struct);
void DIC_InitStructDict(DIC_Dict *Struct);

void DIC_DestroyLinkList(DIC_LinkList *LinkList);
void DIC_DestroyDict(DIC_Dict *Dict);

HAS_Hash *_DIC_HashTable = NULL;
size_t _DIC_DictCount = 0;

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

    // Create hash if needed
    extern HAS_Hash *_DIC_HashTable;
    extern size_t _DIC_DictCount;
    
    if (_DIC_HashTable == NULL)
    {
        _DIC_HashTable = HAS_CreateHash(1, 0);

        if (_DIC_HashTable == NULL)
        {
            _DIC_AddErrorForeign(_DIC_ERRORID_CREATEDIC_HASH, HAS_GetError(), _DIC_ERRORMES_CREATEHASH);
            DIC_DestroyDict(Dict);
            return NULL;
        }
    }

    ++_DIC_DictCount;

    return Dict;
}

bool DIC_AddItem(DIC_Dict *Dict, const char *Key, void *Value, size_t ValueLength, DIC_Mode Mode)
{
    extern HAS_Hash *_DIC_HashTable;
    extern size_t _DIC_DictCount;

    if (_DIC_HashTable == NULL)
    {
        _DIC_SetError(_DIC_ERRORID_ADDITEM_HASHTABLE, _DIC_ERRORMES_NOHASHTABLE, _DIC_DictCount);
        return NULL;
    }

    // Hash the key
    size_t KeyLength = strlen(Key);
    uint64_t HashKey = HAS_HashValue(_DIC_HashTable, (uint8_t *)Key, KeyLength);

    // Find the position of the item
    DIC_LinkList **ItemPos = Dict->list + HashKey % Dict->length;

    while (*ItemPos != NULL)
    {
        // Check if it is a dublicate
        if (strcmp((*ItemPos)->key, Key) == 0)
            break;

        // Get the next item
        ItemPos = &(*ItemPos)->next;
    }

    // Copy the value
    void *CopyValue = Value;

    if (Mode == DIC_MODE_COPY)
    {
        CopyValue = malloc(ValueLength);

        if (CopyValue == NULL)
        {
            _DIC_AddErrorForeign(_DIC_ERRORID_ADDITEM_MALLOCVALUE, strerror(errno), _DIC_ERRORMES_MALLOC, ValueLength);
            return false;
        }

        memcpy(CopyValue, Value, ValueLength);
    }

    // If it did not find the item, create a new item
    if (*ItemPos == NULL)
    {
        // Copy the key
        char *CopyKey = (char *)malloc(sizeof(char) * (KeyLength + 1));

        if (CopyKey == NULL)
        {
            _DIC_AddErrorForeign(_DIC_ERRORID_ADDITEM_MALLOCKEY, strerror(errno), _DIC_ERRORMES_MALLOC, sizeof(char) * (KeyLength + 1));
            if (Mode == DIC_MODE_COPY)
                free(CopyValue);
            return false;
        }

        strcpy(CopyKey, Key);

        DIC_LinkList *NewItem = (DIC_LinkList *)malloc(sizeof(DIC_LinkList));

        if (NewItem == NULL)
        {
            _DIC_AddErrorForeign(_DIC_ERRORID_ADDITEM_MALLOCITEM, strerror(errno), _DIC_ERRORMES_MALLOC, sizeof(DIC_LinkList));
            free(CopyKey);
            if (Mode == DIC_MODE_COPY)
                free(CopyValue);
            return false;
        }

        DIC_InitStructLinkList(NewItem);

        // Set values
        NewItem->key = CopyKey;
        *ItemPos = NewItem;
    }

    // Remove old value
    if (!(*ItemPos)->pointer && (*ItemPos)->value != NULL)
        free((*ItemPos)->value);

    (*ItemPos)->value = CopyValue;
    (*ItemPos)->pointer = (Mode == DIC_MODE_POINTER);

    return true;
}

void *DIC_GetItem(DIC_Dict *Dict, const char *Key)
{
    extern HAS_Hash *_DIC_HashTable;
    extern size_t _DIC_DictCount;

    if (_DIC_HashTable == NULL)
    {
        _DIC_SetError(_DIC_ERRORID_GETITEM_HASHTABLE, _DIC_ERRORMES_NOHASHTABLE, _DIC_DictCount);
        return NULL;
    }

    // Hash the key
    size_t KeyLength = strlen(Key);
    uint64_t HashKey = HAS_HashValue(_DIC_HashTable, (uint8_t *)Key, KeyLength);

    // Find the item
    DIC_LinkList **ItemPos = Dict->list + HashKey % Dict->length;

    while (*ItemPos != NULL)
    {
        // Check if it found it
        if (strcmp((*ItemPos)->key, Key) == 0)
            return (*ItemPos)->value;

        // Get the next item
        ItemPos = &(*ItemPos)->next;
    }

    _DIC_SetError(_DIC_ERRORID_GETITEM_NOITEM, _DIC_ERRORMES_NOITEM);
    return NULL;
}

bool DIC_RemoveItem(DIC_Dict *Dict, const char *Key)
{
    extern HAS_Hash *_DIC_HashTable;
    extern size_t _DIC_DictCount;

    if (_DIC_HashTable == NULL)
    {
        _DIC_SetError(_DIC_ERRORID_REMOVEITEM_HASHTABLE, _DIC_ERRORMES_NOHASHTABLE, _DIC_DictCount);
        return false;
    }

    // Hash the key
    size_t KeyLength = strlen(Key);
    uint64_t HashKey = HAS_HashValue(_DIC_HashTable, (uint8_t *)Key, KeyLength);

    // Find the item
    DIC_LinkList **ItemPos = Dict->list + HashKey % Dict->length;

    while (*ItemPos != NULL)
    {
        // Check if it found it
        if (strcmp((*ItemPos)->key, Key) == 0)
            break;

        // Get the next item
        ItemPos = &(*ItemPos)->next;
    }

    // Make sure that it found something
    if (*ItemPos == NULL)
    {
        _DIC_SetError(_DIC_ERRORID_REMOVEITEM_NOITEM, _DIC_ERRORMES_NOITEM);
        return false;
    }

    // Remove the item
    DIC_LinkList *NextList = (*ItemPos)->next;
    (*ItemPos)->next = NULL;

    DIC_DestroyLinkList(*ItemPos);
    *ItemPos = NextList;

    return true;
}

bool DIC_CheckItem(DIC_Dict *Dict, const char *Key)
{
    extern HAS_Hash *_DIC_HashTable;
    extern size_t _DIC_DictCount;

    if (_DIC_HashTable == NULL)
    {
        _DIC_SetError(_DIC_ERRORID_CHECKITEM_HASHTABLE, _DIC_ERRORMES_NOHASHTABLE, _DIC_DictCount);
        return false;
    }

    // Hash the key
    size_t KeyLength = strlen(Key);
    uint64_t HashKey = HAS_HashValue(_DIC_HashTable, (uint8_t *)Key, KeyLength);

    // Find the item
    DIC_LinkList **ItemPos = Dict->list + HashKey % Dict->length;

    while (*ItemPos != NULL)
    {
        // Check if it found it
        if (strcmp((*ItemPos)->key, Key) == 0)
            return true;

        // Get the next item
        ItemPos = &(*ItemPos)->next;
    }

    return false;
}

void DIC_InitStructLinkList(DIC_LinkList *Struct)
{
    Struct->key = NULL;
    Struct->value = NULL;
    Struct->pointer = true;
    Struct->next = NULL;
}

void DIC_InitStructDict(DIC_Dict *Struct)
{
    Struct->list = NULL;
    Struct->length = 0;
}

void DIC_DestroyLinkList(DIC_LinkList *LinkList)
{
    // Destroy the key
    if (LinkList->key != NULL)
        free(LinkList->key);

    if (!LinkList->pointer && LinkList->value != NULL)
        free(LinkList->value);

    if (LinkList->next != NULL)
        DIC_DestroyLinkList(LinkList->next);

    free(LinkList);
}

void DIC_DestroyDict(DIC_Dict *Dict)
{
    // Destroy the dict
    if (Dict->list != NULL)
    {
        for (DIC_LinkList **List = Dict->list, **EndList = Dict->list + Dict->length; List < EndList; ++List)
            if (*List != NULL)
                DIC_DestroyLinkList(*List);

        free(Dict->list);
    }

    free(Dict);

    // Destroy the hash if needed
    extern HAS_Hash *_DIC_HashTable;
    extern size_t _DIC_DictCount;

    if (_DIC_DictCount == 0)
    {
        _DIC_SetError(_DIC_ERRORID_DESTROYDICT_NODICT, _DIC_ERRORMES_WRONGDICTCOUNT);
        return;
    }

    --_DIC_DictCount;

    // Destroy hash
    if (_DIC_DictCount == 0 && _DIC_HashTable != NULL)
    {
        HAS_DestroyHash(_DIC_HashTable);
        _DIC_HashTable = NULL;
    }
}

#endif