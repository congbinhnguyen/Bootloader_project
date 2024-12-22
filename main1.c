// main.c
#include <stdio.h>
#include "queue.h"

int main() {
    Queue* q = createQueue();

    enqueue(q, 'A');
    enqueue(q, 'B');
    enqueue(q, 'C');
    enqueue(q, 'D');
    
    displayQueue(q);// ham nay chi de test khong dung trong bai 
 
    uint8_t removed = dequeue(q);
    printf("dequeue: %c\n", removed);

    displayQueue(q);

    freeQueue(q);

    return 0;
}

