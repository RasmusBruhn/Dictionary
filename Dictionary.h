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
    _DIC_ERRORID_REMOVEITEM_NOITEM = 0x600060201,
    _DIC_ERRORID_ADDLIST_ADDITEM = 0x600070200,
    _DIC_ERRORID_COPYDICT_CREATE = 0x600080200,
    _DIC_ERRORID_COPYDICT_MALLOCLINK = 0x600080201,
    _DIC_ERRORID_COPYDICT_MALLOCKEY = 0x600080202,
    _DIC_ERRORID_COPYDICT_MALLOCVALUE = 0x600080203
};

#define _DIC_ERRORMES_MALLOC "Unable to allocate memory (Size: %lu)"
#define _DIC_ERRORMES_CREATEHASH "Unable to create hash"
#define _DIC_ERRORMES_WRONGDICTCOUNT "Attempting to destroy dict, but none is supposed to exist"
#define _DIC_ERRORMES_NOHASHTABLE "No hash table is available (Expected number of dicts: %lu)"
#define _DIC_ERRORMES_NOITEM "Unable to locate item"
#define _DIC_ERRORMES_ADDITEM "Unable to add item"
#define _DIC_ERRORMES_CREATEDICT "Unable to create new dict"

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
    char *key; // The key for the item
    void *value; // A pointer to the value
    size_t size; // The size of the value, only used if pointer is false
    bool pointer; // If it is false then it contains a pointer to private information which must be freed when dict is destroyed
    DIC_LinkList *next; // The next element in the list
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
// Value: A pointer to the value to store
// ValueLength: The size of the value data, only used if mode is not DIC_MODE_POINTER
// Mode: If DIC_MODE_POINTER, then it will just save the pointer, if DIC_INSERT, then it will save the pointer and free it when destroying the dict, if DIC_COPY, then it will copy the value pointet to
bool DIC_AddItem(DIC_Dict *Dict, const char *Key, void *Value, size_t ValueLength, DIC_Mode Mode);

// Adds a list of items to a dictionary, upon failure it may leave new items in the dict
// Dict: The dictionary to add the item to
// Keys: The keys for the items
// Count: The number of items
// Values: A pointer to the values to store
// ValueLengths: The size of the value data, only used if mode is not DIC_MODE_POINTER
// Mode: If DIC_MODE_POINTER, then it will just save the pointer, if DIC_INSERT, then it will save the pointer and free it when destroying the dict, if DIC_COPY, then it will copy the value pointet to
bool DIC_AddList(DIC_Dict *Dict, const char **Keys, size_t Count, void **Values, const size_t *ValueLengths, DIC_Mode Mode);

// Remove an item from a dictionary
// Dict: The dictionary to remove an item from
// Key: The key for the item
bool DIC_RemoveItem(DIC_Dict *Dict, const char *Key);

// Get an item from a dictionary
// Dict: The dictionary to remove an item from
// Key: The key for the item
void *DIC_GetItem(DIC_Dict *Dict, const char *Key);

// Checks if an item exists in a dictionary
// Dict: The dictionary to remove an item from
// Key: The key for the item
bool DIC_CheckItem(DIC_Dict *Dict, const char *Key);

// Copies a dictionary
// Dict: The dict to copy
DIC_Dict *DIC_CopyDict(DIC_Dict *Dict);

// Returns the number of elements in the dictionary
// Dict: The dict to get the length of
size_t DIC_DictLength(DIC_Dict *Dict);

void DIC_InitLinkList(DIC_LinkList *Struct);
void DIC_InitDict(DIC_Dict *Struct);

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
    DIC_InitDict(Dict);

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

        DIC_InitLinkList(NewItem);

        // Set values
        NewItem->key = CopyKey;
        *ItemPos = NewItem;
    }

    // Remove old value
    if (!(*ItemPos)->pointer && (*ItemPos)->value != NULL)
        free((*ItemPos)->value);

    (*ItemPos)->value = CopyValue;
    (*ItemPos)->pointer = (Mode == DIC_MODE_POINTER);
    (*ItemPos)->size = ValueLength;

    return true;
}

bool DIC_AddList(DIC_Dict *Dict, const char **Keys, size_t Count, void **Values, const size_t *ValueLengths, DIC_Mode Mode)
{
    // Setup ValueLength if not needed
    size_t Length = 0;

    if (Mode == DIC_MODE_POINTER)
        ValueLengths = &Length;

    // Go through all of the items and add them
    for (const char **NewKeys = Keys, **EndKeys = Keys + Count; NewKeys < EndKeys; ++NewKeys, ++Values)
    {
        // Add item
        if (!DIC_AddItem(Dict, *NewKeys, *Values, *ValueLengths, Mode))
        {
            _DIC_AddError(_DIC_ERRORID_ADDLIST_ADDITEM, _DIC_ERRORMES_ADDITEM);
            return false;
        }

        // Increase the value length
        if (Mode != DIC_MODE_POINTER)
            ++ValueLengths;
    }

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

DIC_Dict *DIC_CopyDict(DIC_Dict *Dict)
{
    // Create a new dict
    DIC_Dict *NewDict = DIC_CreateDict(Dict->length);

    if (NewDict == NULL)
    {
        _DIC_AddError(_DIC_ERRORID_COPYDICT_CREATE, _DIC_ERRORMES_CREATEDICT);
        return NULL;
    }

    // Go through and copy all of the items
    for (DIC_LinkList **SrcList = Dict->list, **DstList = NewDict->list, **EndList = Dict->list + Dict->length; SrcList < EndList; ++SrcList, ++DstList)
        for (DIC_LinkList *SrcLink = *SrcList, **DstLink = DstList; SrcLink != NULL; SrcLink = SrcLink->next, DstLink = &(*DstLink)->next)
        {
            // Create new LinkList
            DIC_LinkList *NewLink = (DIC_LinkList *)malloc(sizeof(DIC_LinkList));

            if (NewLink == NULL)
            {
                _DIC_AddErrorForeign(_DIC_ERRORID_COPYDICT_MALLOCLINK, strerror(errno), _DIC_ERRORMES_MALLOC, sizeof(DIC_LinkList));
                DIC_DestroyDict(NewDict);
                return NULL;
            }

            DIC_InitLinkList(NewLink);

            // Copy key
            NewLink->key = (char *)malloc(sizeof(char) * (strlen(SrcLink->key) + 1));

            if (NewLink->key == NULL)
            {
                _DIC_AddErrorForeign(_DIC_ERRORID_COPYDICT_MALLOCKEY, strerror(errno), _DIC_ERRORMES_MALLOC, sizeof(char) * (strlen(SrcLink->key) + 1));
                DIC_DestroyDict(NewDict);
                DIC_DestroyLinkList(NewLink);
                return NULL;
            }

            strcpy(NewLink->key, SrcLink->key);

            // Copy value
            if (SrcLink->pointer)
                NewLink->value = SrcLink->value;

            else
            {
                NewLink->value = malloc(SrcLink->size);

                if (NewLink->value == NULL)
                {
                    _DIC_AddErrorForeign(_DIC_ERRORID_COPYDICT_MALLOCVALUE, strerror(errno), _DIC_ERRORMES_MALLOC, SrcLink->size);
                    DIC_DestroyDict(NewDict);
                    DIC_DestroyLinkList(NewLink);
                    return NULL;
                }

                memcpy(NewLink->value, SrcLink->value, SrcLink->size);
            }

            NewLink->pointer = SrcLink->pointer;
            NewLink->size = SrcLink->size;

            // Add to dict
            *DstLink = NewLink;
        }

    return NewDict;
}

size_t DIC_DictLength(DIC_Dict *Dict)
{
    size_t Length = 0;

    // Go through the entire dict
    for (DIC_LinkList **List = Dict->list, **EndList = Dict->list + Dict->length; List < EndList; ++List)
        for (DIC_LinkList *Link = *List; Link != NULL; Link = Link->next)
            ++Length;

    return Length;
}

void DIC_InitLinkList(DIC_LinkList *Struct)
{
    Struct->key = NULL;
    Struct->value = NULL;
    Struct->size = 0;
    Struct->pointer = true;
    Struct->next = NULL;
}

void DIC_InitDict(DIC_Dict *Struct)
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