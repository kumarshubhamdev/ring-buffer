#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include<linux/string.h>
#include "ioctl.h"
#include "queue.h" //declaration and definition of Cicular Buffer

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shubham Kumar");
MODULE_DESCRIPTION("Dynamic Circular Buffer with Blocking");

//IOCTL command Macros , as stated in question's requirement
#define SET_SIZE_OF_QUEUE _IOW('a', 'a', int)
#define PUSH_DATA _IOW('a', 'b', data_t )
#define POP_DATA  _IOW('a', 'c', data_t )
#define PRINT _IOW('a','d',data_t)



//Global Queue Node;
static queue_t *q;  //shared by all the processes




/* Device Major Number */
#define MYMAJOR 64

DECLARE_WAIT_QUEUE_HEAD(wq);//Static Wait Queue Head initialization
static DEFINE_MUTEX(q_mutex); // Define a static mutex



//Minimal Static Function
static int driver_open(struct inode *device_file, struct file *instance) {
    printk("Dynamic Cicular Buffer Module - open was called!\n");
    return 0;
}


//Minimal Close Function
static int driver_close(struct inode *device_file, struct file *instance) {
    printk("Dynamic Circular Buffer - close was called!\n");
    return 0;
}


/*


IOCTL function to handle PUSH , POP , SET SIZE COMMANDS


*/

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int ret; //return code
    data_t *copy; //data_t to store user passed data 
    switch (cmd) { //switch cases 
    
        case SET_SIZE_OF_QUEUE://set size functionality
            
            mutex_lock(&q_mutex); 
            int length;//declaring length variable to store the length of buffer to initalize with
            if (copy_from_user(&length, (int __user *)arg, sizeof(int))) //copy_from_user(), as direct access to pointer is DISCOURAGED!
            return -1;//not successful
            
            //resizing logic
            if(q!=NULL) //if q is null it means queue is not initialized yet , skip
            {
        if(length<q->size){printk("WILL LEAD TO DATA CORRUPTION NEW SIZE IS SMALLER THAN OLD SIZE\n"); mutex_unlock(&q_mutex); return -1;}
               queue_t* new_q = kmalloc(sizeof(queue_t),GFP_KERNEL); //allocate new size queue
               if(new_q==NULL)//if allocation failed , return ERROR and print kernel level log 
               {
                  printk("COULD NOT RESET SIZE\n"); mutex_unlock(&q_mutex); return -1; //unlock the mutex before returning, else race conidtion
               }
               
               
               
               new_q->size=length;//new length would be userd passed 
               new_q->start=q->start;//start index would be same 
               new_q->end=q->end;//end index would be same
               new_q->buffer=kmalloc(sizeof(data_t)*length,GFP_KERNEL); //allocated buffer
               
               
               printk("MALLOCING\n");  //kernel level log
               for(int i=0;i<q->size;i++){//copy the old data into new queue
               if(q->buffer[i]._data!=NULL) //check if data is present
               {
               new_q->buffer[i].length=q->buffer[i].length; 
               new_q->buffer[i]._data=kmalloc(sizeof(char)*(q->buffer[i].length),GFP_KERNEL);//allocated space for new data
               memcpy((new_q->buffer[i]._data),(q->buffer[i]._data),sizeof(q->buffer[i].length)); //copy the data 
                 kfree(q->buffer[i]._data);//free the old memory
               }
               else
               {
                  new_q->buffer[i].length=0; //initlaize the new region if empty
                  new_q->buffer[i]._data=NULL;//initalize to null as not data here in previous too
               }
             
                    
               }
               
               
               if(q!=NULL) //fre the old memory region of data_t* buffer;
               kfree(q);
               
               q=new_q;//point to the new memory location
               printk("ASSIGNED SIZE:%d\n",length);//kernel level print log as successful assigning
               mutex_unlock(&q_mutex);//unlock the mutex
               return 1; //return success
            }      
            ret = set_size(&q, length);//1 is returned is successful
            mutex_unlock(&q_mutex);
            if (ret == -1)
                return -1;
                break;
                
                
        case PUSH_DATA://PUSH_DATA functionality
            copy = kmalloc(sizeof(data_t), GFP_KERNEL);//initializing copy data region to hold user sent data
            if (!copy)//if copy is unsuccessful
                return -1;
            if (copy_from_user(copy, (data_t __user *)arg, sizeof(data_t))) {
                if(copy!=NULL)
                kfree(copy);//is copy is failed,free the memory space;
                return -1;
            }
           wait_event(wq,!isFull(q)); //block the PUSH_DATA callee till that data is Full;
           mutex_lock(&q_mutex);
           push_data(&q, copy);//PUSH_DATA when Buffer is not full
           mutex_unlock(&q_mutex);
           wake_up(&wq);//wakeup the Process waiting
           break;
           
           
        case POP_DATA://POP_DATA functionality
            wait_event(wq,!isEmpty(q));//Block the POP_DATA callee till the Buffer is Empty
            mutex_lock(&q_mutex);
            pop(&q);//POP when the Buffer is not Empty
            mutex_unlock(&q_mutex);
            wake_up(&wq);//Wake up the Processes blocked
            break;
            
            
        case PRINT://Debug Print command to print the queue
          mutex_lock(&q_mutex);
          print_debug(q);//uses printk() function , visible in kernel log only , not in userspace , use dmesg to see
          mutex_unlock(&q_mutex);
          break;
          
          
        default://no registered command matched
        return -1;
        }
    return 0;
}


//Assigning function to fops
/* File Operations */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = driver_open,
    .release = driver_close,
    .unlocked_ioctl = my_ioctl
};


//initialization code , Minimal
 int __init ModuleInit(void) {
    int retval;

    printk("Hello, Kernel!\n");
    

    /* Register device number */
    retval = register_chrdev(MYMAJOR, "DYNAMIC CICRCULAR BUFFER", &fops);
    if (retval < 0) {
        printk("Could not register device number!\n");
        return -1;
    }

    printk("DYNAMIC CIRCULAR BUFFER - registered Major: %d\n", MYMAJOR);

    q=NULL;//setting q as NULL , initially

    return 0;
}


//Cleanup Code
 void __exit ModuleExit(void) {
    unregister_chrdev(MYMAJOR, "DYNAMIC CIRCULAR BUFFER");
    printk("Goodbye, Kernel\n");
    
    
    //freeing the main queue Node;
    if(q!=NULL)kfree(q);
    q=NULL;

}

module_init(ModuleInit);
module_exit(ModuleExit);
