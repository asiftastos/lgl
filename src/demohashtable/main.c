#include "lgl/demo.h"
#include <time.h>

static const unsigned long CAPACITY = 100;

typedef struct HTitem
{
    char* key;
    char* value;
}HTitem;

static Demo* d = NULL;

//simple, will have collisions
static unsigned long hash_func(const char* str)
{
    unsigned long i = 0;
    for (size_t j = 0; str[j]; j++)
    {
        i += str[j];
    }
    return i % CAPACITY; //make sure i is inside CAPACITY
}

static unsigned long hash_func_better(const char* str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash % CAPACITY;
}

static HTitem* items = NULL;

static void addItem(const char* key, const char* value)
{
    unsigned long index = hash_func_better(key);
    HTitem* i = &items[index];
    if(i && i->key && strcmp(i->key, key) == 0)
    {
        printf("Conflict with item [%lu]: %s - %s\n", index, i->key, i->value);
        return;
    }

    HTitem* item = (HTitem*)calloc(1, sizeof(HTitem));
    item->key = (char*)calloc(strlen(key), sizeof(char));
    strcpy(item->key, key);
    item->value = (char*)calloc(strlen(value), sizeof(char));
    strcpy(item->value, value);
    items[index] = *item;
}

static HTitem* getItem(const char* key)
{
    unsigned long index = hash_func_better(key);
    return &items[index];
}

static void printItem(const char* key)
{
    unsigned long index = hash_func_better(key);
    HTitem item = items[index];
    printf("Item [%lu]: %s - %s\n", index, item.key, item.value);
}

static void printItems()
{
    for (size_t i = 0; i < CAPACITY; i++)
    {
        HTitem* item = &items[i];
        if(item)
        {
            printf("Item [%zu]: %s - %s\n", i, item->key, item->value);
        }else
        {
            printf("Item [%zu]: %s - %s\n", i, "NULL", "NULL");
        }
    }
}

int Random(int min, int max)
{
    time_t t;
    srand((unsigned int)time(&t));
    return (rand()%(abs(max - min) + 1) + min);
}

void init()
{
    printf("Demo hashtable initialized\n");
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    items = (HTitem*)calloc(CAPACITY, sizeof(HTitem));

    addItem("Air", "Air Type");
    addItem("Sand", "Sand Type");
    addItem("Stone", "Stone Type");
    addItem("Water", "Water Type");
    addItem("Lava", "Lava Type");
    addItem("Iron", "Iron Type");
    addItem("Copper", "Copper Type");
    addItem("Gravel", "Gravel Type");

    int ar[5] = {1,2,3,4,5};
    int r = Random(1,4);
    printf("Random: %d\n", r);
    int index = r;
    for(int i = 0; i < 4; i++)
    {
        printf("Index: %d, Value: %d\n", i, ar[index]);
        index = (index + 1) % 5;
    }
}

void terminate()
{
    //printItems();

    if(items)
        free((void*)items);

    printf("Demo hashtable terminated\n");
}

void update()
{
}

void render()
{
}

int main(void)
{
    d = demoCreate(init, terminate, update, render);
    demoRun();
    demoDestroy();

    return 0;
}
