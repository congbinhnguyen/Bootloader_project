// queue.c
#include "queue.h"
#include <stdlib.h>

// Function to create a new queue
Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q)
    {
        exit(EXIT_FAILURE);
    }
    q->front = q->rear = NULL;
    return q;
}

// Function to check if the queue is empty
int isEmpty(Queue* q) {
    return (q->front == NULL);
}

// Function to add an element to the queue (enqueue)
void enqueue(Queue* q, uint8_t value) {
    Node* temp = (Node*)malloc(sizeof(Node));
    if (!temp)
    {
        exit(EXIT_FAILURE);
    }
    temp->data = value;
    temp->next = NULL;
    if (q->rear == NULL)
    {
        q->front = q->rear = temp;
        return;
    }
    q->rear->next = temp;
    q->rear = temp;
}

// Function to remove an element from the queue (dequeue)
uint8_t dequeue(Queue* q)
{
    if (isEmpty(q))
    {
        exit(EXIT_FAILURE);
    }
    Node* temp = q->front;
    uint8_t data = temp->data;
    q->front = q->front->next;

    // If the queue becomes empty after dequeue, update the rear pointer
    if (q->front == NULL)
    {
        q->rear = NULL;
    }
    free(temp);
    return data;
}

// Function to display the contents of the queue
void displayQueue(Queue* q) {
    if (isEmpty(q)) {
        return;
    }
    Node* temp = q->front;
    while (temp != NULL)
    {
        temp = temp->next;
    }
}

// Function to free the memory allocated for the queue
void freeQueue(Queue* q)
{
    while (!isEmpty(q))
    {
        dequeue(q);
    }
    free(q);
}

