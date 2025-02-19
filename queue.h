#ifndef _QUEUE_
#define _QUEUE_

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>


//IOCTL COMMANDS DECLARATION , structure as followed in question's requirement
#define SET_SIZE_OF_QUEUE _IOW('a', 'a', int)
#define PUSH_DATA _IOW('a', 'b', data_t )
#define POP_DATA  _IOW('a', 'c', data_t )
#define PRINT _IOW('a','d',data_t)

typedef struct data data_t;  //typdefining the data struct
typedef struct queue queue_t; // typedefining the queue struct
typedef struct master master_t; /* typedefining the master struct , could be used later to encapsulate the Queue at later stage
so a mutex lock could be maintained i.e. queueNode head would be inside it and any operation would need to lock on master first

struct master{
struct qeueue_t* queueHead;
mutex lock;
};
//lock on master to access the queue
*/


//defining struct data as given in the requirement
struct data{
   int length;
   char* _data;
};



//defining the queue structure
struct queue{
data_t* buffer; //buffer array of type data
int size; //size of the bufer
int start; //start index of the buffer
int end; //end index of the buffer
};




//Functions declaration
int set_size(queue_t** q,int size);//passing the queue_t as **(double pointer) to achieve pass by reference 
int push_data(queue_t** q,data_t *data);//passing the queue_t as **(double pointer) to achieve pass by reference 
int pop(queue_t** q);//passing the queue_t as **(double pointer) to achieve pass by reference and reflect changes on queueNode passed
int isFull(queue_t *q);//returns 1 on success 0 on not
int isEmpty(queue_t *q);//returns1 on success and 0 on not


void print_debug(queue_t* q);//Print function to see the current Buffer status , printk() , kernel level log


int set_size(queue_t** q,int size) //set_size definition -> takes queue_t** to achieve pass by reference , size->size of the buffer
{
     
     //Initialization of queue_t node;
     *q=kmalloc(sizeof(queue_t),GFP_KERNEL); //allocation using kmalloc() as kernel level code , not malloc()!
     
     
     
     //allocation success check
     if((*q)==NULL)
     {
     printk("Initialization Failed\n");
     return -1;
     
     }
     
     
     //Initalization of queue_t fields
     (*q)->buffer=kmalloc((sizeof(data_t)*size),GFP_KERNEL);
     
     //buffer allocation success check
     if((*q)->buffer==NULL)
     {
        printk("Queue Buffer was not allocated\n");
        return -1;
     }
     
     
     //initialization of buffer fields
     (*q)->size=size;
     (*q)->start=-1;
     (*q)->end=-1;
     
     
     return 1;

}


int isFull(queue_t *q) //returns 1 on success and 0 on failure
{
   //NULL pointer check
   if(q==NULL)
   {
     printk("NULL Queue pointer passed\n");
     return -1;
   }
   
   
   /*
   [S,____,E]---> for buffer to be full S==((E+1)%SIZE_OF_BUFFER)
   */
   if(((q->end+1)%(q->size))==q->start)return 1;
   
   
   return 0;

}


int isEmpty(queue_t *q)//returns 1 on success and 0 on failure
{
    //NULL pointer check
   if(q==NULL)
   {
     printk("NULL Queue pointer passed\n");
     return -1;
   }
   
   if(q->start==-1)return 1; //Buffer is Empty
   
   return 0; //is not Empty

}

int push_data(queue_t** q,data_t *data)//to push data ,takes queue_t** to pass as reference, data_t*->data to be inserted 
{
   
   //NULL pointer check
   if((*q)==NULL)
   {
     printk("NULL Queue pointer passed\n");
     return -1;
   }
   
   
   if(isFull(*q)){
   printk("Queue is full\n");
   return -1;
   }

   if(isEmpty(*q))
   {
      (*q)->start=0;
      (*q)->end=0;
      
   }


   else
   { 
      (*q)->end=((*q)->end+1)%((*q)->size);
      
   }
   
 //  printk("LEN:%d",data->length);
 
   //Allocation buffer[i]->data field , as this will be a separate memory region , to keep the data persistent 
   
   //i.e. data_t (user) != data_t(buffer)
   // data from user space is COPIED DEEPLY , NO SHALLOW COPY
   
   /*
   for reference
   struct data{
   int length;
   char* data;
   };
   
   */
   
   (*q)->buffer[(*q)->end].length = data->length; //initalizating length
    (*q)->buffer[(*q)->end]._data = kmalloc(data->length * sizeof(char),GFP_KERNEL);//allocating char* data;
    if ((*q)->buffer[(*q)->end]._data == NULL)//if data allocation fails
    {
        printk("Memory allocation for _data failed\n");
        return -1;
    }

    
    
    //copying the data passed
    for (int i = 0; i < data->length; i++)
    {
        (*q)->buffer[(*q)->end]._data[i] = data->_data[i];
    }

   
   
   return 1; //success

}



int pop(queue_t **q)//pop the front,as in real queue(question requirement stated to be the node random , but this breaks queue rule i.e. FIFO) 
{
   if((*q)==NULL) 
   {
     printk("NULL pointer passed\n");
     return -1;
   }
   
   if(isEmpty(*q))
   {
      printk("Queue is Empty!\n");
      return 0;
   
   }
   
    if ((*q)->buffer[(*q)->start]._data != NULL) { //freeing the data region 
    kfree((*q)->buffer[(*q)->start]._data);
    (*q)->buffer[(*q)->start]._data = NULL;
  }

  
  
  (*q)->start=((*q)->start+1)%((*q)->size); //incrementing the start index
  
  if ( (*q)->buffer[(*q)->start]._data == NULL ) { //if last node is popped , queue is empty again and hence the condition set
       (*q)->start = -1;
       (*q)->end = -1;
       return 1;
   }
   
   return 1;


}



void print_debug(queue_t* q) //to print the queue
{

   if(q==NULL)
   {
      printk("Cannot printk a NULL pointer!\n");
      return;
   }
   
   for(int i=0;i<q->size;i++)
   {
     printk("----NODE:%d----\n",i+1);
     
     if(q->buffer[i]._data==NULL){printk("Data Not Found at this location\n"); continue;}//if data region is not present , that is if it is
     //popped earlier and not filled yet , then skip this node 
     
     for(int j=0;j<q->buffer[i].length;j++) //print the region 
     {
       printk("%c",((q->buffer[i])._data[j]));
     }
     printk("\n-----------------\n\n\n");
    

   
   }

   return;
}















#endif