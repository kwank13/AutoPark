/*
	Implement queue to filter front sensor values. sum used to keep track of the sum of all values in the queue to get the average. This is an optimization so we don't need to do go through a for loop 	every time the average() function is called.
*/
#include "frontQueue.h"

float frontBuffer[FRONTQUEUEMAX];
int front = 0;
int rear = -1;
int itemCount = 0;
float sum = 0;

float peek() {
   return frontBuffer[front];
}

bool isEmpty() {
   return itemCount == 0;
}

bool isFull() {
   return itemCount == FRONTQUEUEMAX;
}

int size() {
   return itemCount;
}  

void insert(float data) {

   if(!isFull()) {
	
      if(rear == FRONTQUEUEMAX-1) {
         rear = -1;            
      }       

      sum += data;

      frontBuffer[++rear] = data;
      itemCount++;
   }
}

float removeData() {
   float data = frontBuffer[front++];
	
   if(front == FRONTQUEUEMAX) {
      front = 0;
   }
	
   sum -= data;

   itemCount--;
   return data;
}

void init_queue() {
   int count = 0;


   for (count = 0; count < FRONTQUEUEMAX; count++)
      frontBuffer[count] = 30.0f;

   sum = 30.0f * FRONTQUEUEMAX;
}

float average() {
   return sum/FRONTQUEUEMAX;
}
