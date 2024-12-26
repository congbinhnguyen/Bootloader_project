// queue.h
#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>

// Definition of a node in the queue
typedef struct Node {
    uint8_t data;          // Data stored in the node
    struct Node* next;     // Pointer to the next node
} Node;

// Definition of the queue
typedef struct Queue {
    Node* front;           // Pointer to the front of the queue
    Node* rear;            // Pointer to the rear of the queue
} Queue;

// Function to create a new queue
Queue* createQueue();

// Function to check if the queue is empty
int isEmpty(Queue* q);

// Function to add an element to the queue (enqueue)
void enqueue(Queue* q, uint8_t value);

// Function to remove an element from the queue (dequeue)
uint8_t dequeue(Queue* q);

// Function to display the contents of the queue
void displayQueue(Queue* q);

// Function to free the memory allocated for the queue
void freeQueue(Queue* q);



#endif // QUEUE_H
