/*
 * myQueue.c
 *
 *  Created on: Dec 7, 2015
 *      Author: Eric
 */

#include "myQueue.h"

Queue * createQueue(){
    /* Create a Queue */
    Queue *Q;
    Q = (Queue *)malloc(sizeof(Queue));
    /* Initialise its properties */
    Q->elements = (Msg *)malloc(sizeof(Msg)*CAPACITY);
    Q->size = 0;
    Q->capacity = CAPACITY;
    Q->front = 0;
    Q->rear = -1;
    /* Return the pointer */
    return Q;
}

int dequeue(Queue * Q){
    /* If Queue size is zero then it is empty. So we cannot pop */
    if(Q->size==0)
    {
            //printf("Queue is Empty\n");
            return 1;
    }
    /* Removing an element is equivalent to incrementing index of front by one */
    else
    {
            Q->size--;
            Q->front++;
            /* As we fill elements in circular fashion */
            if(Q->front==Q->capacity)
            {
                    Q->front=0;
            }
    }
    return 0;
}

int enqueue(Queue * Q, Msg element){
    /* If the Queue is full, we cannot push an element into it as there is no space for it.*/
    if(Q->size == Q->capacity)
    {
		//printf("Queue is Full\n");
    	return 1;
    }
    else
    {
		Q->size++;
		Q->rear = Q->rear + 1;
		/* As we fill the queue in circular fashion */
		if(Q->rear == Q->capacity)
		{
				Q->rear = 0;
		}
		/* Insert the element in its rear side */
		Q->elements[Q->rear] = element;
    }
    return 0;
}

Msg front(Queue * Q){
	if(Q->size==0)
	{
			//printf("Queue is Empty\n");
			//exit(0);
	}
	/* Return the element which is at the front*/
	return Q->elements[Q->front];
}

Msg queueGet(Queue * Q){
	Msg M;
	M = front(Q); // get the first element
	dequeue(Q); // pop the first element off of the stack
	return M;
}

int queuePut(Queue * Q, Msg element){
	enqueue(Q, element);
	return 0;
}


