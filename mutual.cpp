#include <iostream>
#include<semaphore.h>
#include<stdio.h>
#include<queue>
#include<pthread.h>
#include <unistd.h>
#define MAX_SIZE 5
# include<bits/stdc++.h>

using namespace std;

int max_size = 5;
int value = 0;

sem_t s_read_write;
sem_t sem_buffer, sem_active_consumer, size_limit;
int rand();
FILE * TFile;

typedef struct
{
    int Front,Rear;
    int number_of_elements;
    int item[MAX_SIZE];
} Queue;
 Queue q;
void init(Queue *q)
{
    q->Front=-1;
    q->Rear=-1;
    q->number_of_elements = 0;
}
int isFull(Queue *q)
{
    if (q->number_of_elements == MAX_SIZE)
        return 1;
    else
        return 0;
}

int isEmpty(Queue *q)
{
    if (q->number_of_elements == 0)
        return 1;
    else
        return 0;
}

int dequeue(Queue *q)
{
    if (!isEmpty(q))
    {
        q->Front = (q->Front + 1) % MAX_SIZE;
        int value = q->item[q->Front];
        q->number_of_elements--;
        return value;
    }
    /*else
    {
        printf("[Underflow] Can't dequeue: Queue is empty!\n");
        exit(1);
    }*/
}
void enqueue(Queue *q, int value)
{
    if (!isFull(q))
    {
        q->Rear = (q->Rear + 1) % MAX_SIZE;
        q->item[q->Rear] = value;
        q->number_of_elements++;
    }
   /* else
    {
        printf("[Overflow] Can't enqueue: Queue is full!\n");
        exit(1); // Error
    }*/
}
void* write(void*dectector_id)
{
   while(1){
        usleep((rand()%10)*1000);
        printf("Counter Thread %d : Recieved a massage \n",(long) dectector_id);
        fprintf(TFile,"Counter Thread %d : Recieved a massage \n",(long) dectector_id);
        printf("Counter Thread %d : Waiting to write \n",(long) dectector_id);
        fprintf(TFile,"Counter Thread %d : Waiting to write \n",(long) dectector_id);
        sem_wait(&s_read_write);
        value++;
        printf("Counter Thread %d : Now adding to counter , counter = %d \n",(long)dectector_id ,value);
        fprintf(TFile,"Counter Thread %d : Now adding to counter , counter = %d \n",(long)dectector_id ,value);
        sem_post(&s_read_write);
    }
}
void* read(void*dectector_id)
{
   while(1){
        usleep((rand()%10)*3000);
        printf("Moniter Thread : waiting to read counter\n");
        fprintf(TFile,"Moniter Thread : waiting to read counter\n");
        int value_read;
        sem_wait(&s_read_write);
        value_read = value;
        value = 0;
        printf("Moniter thread : Value read by the moniter is %d \n",value_read);
        fprintf(TFile,"Moniter thread  : Value read by the moniter is %d \n",value_read);
        sem_post(&s_read_write);
        if(isFull(&q)){
            printf("Moniter Thread : Buffer is Full\n");
            fprintf(TFile,"Moniter Thread  : Buffer is Full\n");
        }
        sem_wait(&size_limit);
        sem_wait(&sem_buffer);
            enqueue(&q,value_read);
            printf("---------------------------------------------\n");
            fprintf(TFile,"---------------------------------------------\n");
            printf("Moniter Thread : add value of %d in position %d \n",value_read,q.Rear);
            fprintf(TFile,"Moniter Thread : add value of %d in position %d \n",value_read,q.Rear);
        sem_post(&sem_buffer);
        sem_post(&sem_active_consumer);
    }
}
void* prod(void*dectector_id){
    while(1){
        usleep((rand()%10)*3000);

        if(isEmpty(&q)){
            printf("Collector Thread :  Buffer is empty \n");
            fprintf(TFile,"Collector Thread : Buffer is empty \n");
        }
        sem_wait(&sem_active_consumer);
        sem_wait(&sem_buffer);
            printf("***************************************\n");
            fprintf(TFile,"***************************************\n");
            int Output = dequeue(&q);
            printf("Collector Thread : Reading value %d from the buffer at position %d \n",Output,q.Front);
            fprintf(TFile,"Collector Thread : Reading value %d from the buffer at position %d \n",Output,q.Front);
        sem_post(&sem_buffer);
        sem_post(&size_limit);
    }
}
int main()
{

int N_mcounter = 5;
    /*printf("Enter the number of mcounter thread :\n");
    int N_mcounter;
    cin>>N_mcounter;
    */

    init(&q);
    sem_init(&s_read_write,1,1);
    sem_init(&sem_buffer,1,1);
    sem_init(&sem_active_consumer,1,0);
    sem_init(&size_limit,1,max_size);

    TFile = fopen ("output.txt","w");

    pthread_t writers[5],mmoniter,mcollector;

        for(int i=0;i<N_mcounter;i++){
            pthread_create(&writers[i],NULL,&write,(void*)i);
        }
    /*pthread_create(&writers[0],NULL,&write,(void*)0);
    pthread_create(&writers[1],NULL,&write,(void*)1);
    pthread_create(&writers[2],NULL,&write,(void*)2);

    pthread_create(&writers[3],NULL,&write,(void*)3);
    pthread_create(&writers[4],NULL,&write,(void*)4);*/

    pthread_create(&mmoniter,NULL,&read,NULL);
    pthread_create(&mcollector,NULL,&prod,NULL);

    for(int i=0;i<N_mcounter;i++){
            pthread_join(writers[i],NULL);
    }

   /* pthread_join(writers[0],NULL);
    pthread_join(writers[1],NULL);
    pthread_join(writers[2],NULL);
    pthread_join(writers[3],NULL);
    pthread_join(writers[4],NULL);*/
    pthread_join(mmoniter,NULL);
    pthread_join(mcollector,NULL);
    return 0;
}
