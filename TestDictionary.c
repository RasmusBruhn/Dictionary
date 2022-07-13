#include <stdio.h>
#include <string.h>
#include "Dictionary.h"

int main(int argc, char **argv)
{
    // Create a dictionary
    DIC_Dict *Dict = DIC_CreateDict(256);

    if (Dict == NULL)
    {
        printf("Unable to create dictionary: %s\n", DIC_GetError());
        return 0;
    }

    // Make sure that it created a hash
    extern HAS_Hash *_DIC_HashTable;
    extern size_t _DIC_DictCount;

    if (_DIC_HashTable == NULL)
    {
        printf("Did not create hash table\n");
        return 0;
    }

    if (_DIC_DictCount != 1)
    {
        printf("Dict count is wrong (should be 1): %lu\n", _DIC_DictCount);
        return 0;
    }

    // Destroy it
    DIC_DestroyDict(Dict);

    // Make sure it cleaned up
    if (_DIC_HashTable != NULL)
    {
        printf("Did not destroy hash table\n");
        return 0;
    }

    if (_DIC_DictCount != 0)
    {
        printf("Dict count is wrong (should be 0): %lu\n", _DIC_DictCount);
        return 0;
    }

    Dict = DIC_CreateDict(256);

    if (Dict == NULL)
    {
        printf("Unable to create dictionary: %s\n", DIC_GetError());
        return 0;
    }

    // Create extra dict
    DIC_Dict *ExtraDict = DIC_CreateDict(256);

    if (_DIC_DictCount != 2)
    {
        printf("Dict count is wrong (should be 2): %lu\n", _DIC_DictCount);
        return 0;
    }

    // Destroy extra dict
    DIC_DestroyDict(ExtraDict);

    if (Dict == NULL)
    {
        printf("Destroyed hash too early: %s\n", DIC_GetError());
        return 0;
    }

    // Add elements to dict
    if (!DIC_AddItem(Dict, "First", "Value1", strlen("Value1") + 1, DIC_MODE_COPY))
    {
        printf("Unable to add element 1 to dict: %s\n", DIC_GetError());
        return 0;
    }

    if (!DIC_AddItem(Dict, "Second", "Value2", strlen("Value2") + 1, DIC_MODE_COPY))
    {
        printf("Unable to add element 2 to dict: %s\n", DIC_GetError());
        return 0;
    }

    if (!DIC_AddItem(Dict, "Third", "Value3", strlen("Value3") + 1, DIC_MODE_COPY))
    {
        printf("Unable to add element 3 to dict: %s\n", DIC_GetError());
        return 0;
    }

    if (!DIC_AddItem(Dict, "Third", "Value4", strlen("Value4") + 1, DIC_MODE_COPY))
    {
        printf("Unable to add element 4 to dict: %s\n", DIC_GetError());
        return 0;
    }

    if (!DIC_AddItem(Dict, "Firstaa", "Value5", strlen("Value5") + 1, DIC_MODE_COPY))
    {
        printf("Unable to add element 5 to dict: %s\n", DIC_GetError());
        return 0;
    }

    char *Value;

    Value = DIC_GetItem(Dict, "First");

    if (Value == NULL)
    {
        printf("Unable to get value 1: %s\n", DIC_GetError());
        return 0;
    }

    printf("Got value 1: %s\n", Value);

    Value = DIC_GetItem(Dict, "Second");

    if (Value == NULL)
    {
        printf("Unable to get value 2: %s\n", DIC_GetError());
        return 0;
    }

    printf("Got value 2: %s\n", Value);

    Value = DIC_GetItem(Dict, "Third");

    if (Value == NULL)
    {
        printf("Unable to get value 3: %s\n", DIC_GetError());
        return 0;
    }

    printf("Got value 3: %s\n", Value);

    Value = DIC_GetItem(Dict, "Firstaa");

    if (Value == NULL)
    {
        printf("Unable to get value 5: %s\n", DIC_GetError());
        return 0;
    }

    printf("Got value 5: %s\n", Value);

    // Check other functions
    if (!DIC_CheckItem(Dict, "First"))
    {
        printf("Could not find element\n");
        return 0;
    }

    // Destroy all dicts
    DIC_DestroyDict(Dict);

    // Check the diversity
    Dict = DIC_CreateDict(8);

    if (Dict == NULL)
    {
        printf("Unable to create dictionary: %s\n", DIC_GetError());
        return 0;
    }

    char Key[9] = "uint64_t";

    for (uint64_t i = 0; i < 100; ++i)
    {
        *(uint64_t *)Key = i;
        DIC_AddItem(Dict, Key, NULL, sizeof(void *), DIC_MODE_POINTER);
    }

    // Print the distribution
    for (DIC_LinkList **List = Dict->list, **EndList = Dict->list + Dict->length; List < EndList; ++List)
    {
        size_t Size = 0;

        for (DIC_LinkList *Link = *List; Link != NULL; Link = Link->next)
            ++Size;

        printf("Element: %lu, Count: %lu\n", (uint64_t)(List - Dict->list), Size);
    }

    DIC_DestroyDict(Dict);

    // Try to add a list
    Dict = DIC_CreateDict(8);

    if (Dict == NULL)
    {
        printf("Unable to create dictionary: %s\n", DIC_GetError());
        return 0;
    }

    char *KeyList[] = {"List1", "List2", "List3", "List4"};
    char *ValueList[] = {"Value1", "Value2", "Value3", "Value4"};

    if (!DIC_AddList(Dict, (const char **)KeyList, 4, (void **)ValueList, NULL, DIC_MODE_POINTER))
    {
        printf("Unable to add list to dictionary: %s\n", DIC_GetError());
        return 0;
    }

    printf("List1: %s\n", DIC_GetItem(Dict, KeyList[0]));
    printf("List2: %s\n", DIC_GetItem(Dict, KeyList[1]));
    printf("List3: %s\n", DIC_GetItem(Dict, KeyList[2]));
    printf("List4: %s\n", DIC_GetItem(Dict, KeyList[3]));

    DIC_DestroyDict(Dict);

    printf("Finished without errors\n");

    return 0;
}