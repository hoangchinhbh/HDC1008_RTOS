/*
 * myQueue.h
 *
 *  Created on: Dec 7, 2015
 *      Author: Eric
 */

#include <stdint.h>

#ifndef MYQUEUE_H_
#define MYQUEUE_H_

#define CAPACITY 2

typedef struct hdcMsg {
	uint16_t * temperatureBuffer;
	uint16_t * humidityBuffer;
	uint16_t dataCount;
} Msg;

typedef struct myQueue {
	int capacity;
	int size;
	int front;
	int rear;
	Msg * elements;
} Queue;

Queue * createQueue();
int dequeue(Queue * Q);
int enqueue(Queue * Q, Msg element);
Msg front(Queue * Q);
Msg queueGet(Queue * Q);
int queuePut(Queue * Q, Msg element);

#endif /* MYQUEUE_H_ */
