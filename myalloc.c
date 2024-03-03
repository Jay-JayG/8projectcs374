#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>

#define ALIGNMENT 16   // Must be power of 2
#define GET_PAD(x) ((ALIGNMENT - 1) - (((x) - 1) & (ALIGNMENT - 1)))
#define PADDED_SIZE(x) ((x) + GET_PAD(x))

typedef char Byte16[16];

struct block {
    int size;            // Size in bytes
    int in_use;          // Boolean
    struct block *next;
};

struct block *head;
int mmapped = 0;

void *myalloc(int size) {
    if (!mmapped) {
    void *heap = mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
    mmapped = 1;
    struct block *head_node = heap;
    head_node->size = 1024 - PADDED_SIZE(sizeof(struct block));
    head_node->in_use = 0;
    head_node->next = NULL;
    head = heap;
    }
    struct block *temp = head;
    while (temp != NULL) {
        if (temp->in_use == 0 && temp->next != NULL && temp->size >= PADDED_SIZE(size)) {
            temp->in_use = 1;
            return (void *)((void *)temp + PADDED_SIZE(sizeof(struct block)));
        }
        if (temp->size > PADDED_SIZE(size + sizeof(struct block) + sizeof(Byte16)) && temp->in_use == 0) {
            struct block *new_node = (void *)temp + (PADDED_SIZE(size) + sizeof(struct block));
            new_node->size = temp->size - (PADDED_SIZE(size) + sizeof(struct block));
            new_node->in_use = 0;
            temp->size = PADDED_SIZE(size);
            temp->in_use = 1;
            if (temp->next) {
                new_node->next = temp->next;
                temp->next = new_node;
            } else {
                temp->next = new_node;
            }
            return (void *)((void *)temp + PADDED_SIZE(sizeof(struct block)));
        }
        temp = temp->next;
        if (temp == NULL) {
            return NULL;
        }
    }
    return (void *)((void *)temp + PADDED_SIZE(sizeof(struct block)));
}

void myfree(void *p) {
    p -= PADDED_SIZE(sizeof(struct block));
    struct block *header = (struct block *)p;
    header->in_use = 0;
}

void print_data(void)
{
    struct block *b = head;

    if (b == NULL) {
        printf("[empty]\n");
        return;
    }

    while (b != NULL) {
        // Uncomment the following line if you want to see the pointer values
        //printf("[%p:%d,%s]", b, b->size, b->in_use? "used": "free");
        printf("[%d,%s]", b->size, b->in_use? "used": "free");
        if (b->next != NULL) {
            printf(" -> ");
        }

        b = b->next;
    }

    printf("\n");
}

void main(void) {
}