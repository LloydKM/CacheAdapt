#ifndef CACHE_LAYER_H
#define CACHE_LAYER_H

#include <stdlib.h>
#include <stdio.h>

#define SIZE 20

// 
struct data_item
{
    void *data;
    int key;
};

struct data_item *hash_array[SIZE];
struct data_item *dummy_item;

int
hash_code(int key)
{
    return key % SIZE;
}

struct data_item *
search(int key)
{
    // get the hash
    int hash_index = hash_code(key);

    //  move in array until an empty
    while(hash_array[hash_index] != NULL) 
    {
        if (hash_array[hash_index]->key == key)
            return hash_array[hash_index];
        
        // go to the next entry
        ++hash_index;

        // wrap around the table
        hash_index %= SIZE;
    }

    return NULL;
}

void
insert(int key, void *data)
{
    struct data_item *item = (struct data_item *) malloc(sizeof(struct data_item));
    item->data = &data;
    item->key = key;

    // get the hash
    int hash_index = hash_code(key);

    // move in array until an empty or deleted cell
    while (hash_array[hash_index] != NULL && hash_array[hash_index]->key != -1)
    {
        // go to next cell
        ++hash_index;

        // wrap around the table
        hash_index %= SIZE; 
    }

    hash_array[hash_index] = item;
}

struct data_item *
delete(struct data_item *item) 
{
    int key = item->key;

    // get the hash
    int hash_index = hash_code(key);

    // move in array until an empty
    while(hash_array[hash_index] != NULL)
    {
        if (hash_array[hash_index]->key == key)
        {
            struct data_item *temp = hash_array[hash_index];

            // assign a dummy item at deleted position
            hash_array[hash_index] = dummy_item;
            return temp;
        }

        //go to the next entry
        ++hash_index;

        // wrap around the table
        hash_index %= SIZE;
    }

    return NULL;
}

void
display()
{
    int i = 0;

    for (i = 0; i < SIZE; i++)
    {
        if (hash_array[i] != NULL)
            printf(" (%d,%d)", hash_array[i]->key, hash_array[i]->data);
        else    
            printf(" ~~ ");
    }

    printf("\n");
}


#endif