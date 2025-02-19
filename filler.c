//filler.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include<string.h>

#define DRIVER_NAME "/dev/shubham"
struct data{
   int length;
   char* _data;
};
typedef struct data data_t;


#define SET_SIZE_OF_QUEUE _IOW('a', 'a', int)
#define PUSH_DATA _IOW('a', 'b', data_t )
#define POP_DATA  _IOW('a', 'c', data_t )
#define PRINT _IOW('a','d',data_t)


int main() {

   
   
   
   
 int fd = open(DRIVER_NAME, O_RDWR);

	if(fd == -1) {
		printf("Opening was not possible!\n");
		return -1;
	}

   char test[]="xyz";
  data_t* d=(data_t*)malloc(sizeof(data_t));
  d->length=3;
  d->_data=(char*)malloc(sizeof(char)*3);
    
  memcpy(d->_data,test,3);
  

  int ret1=ioctl(fd,PUSH_DATA,&(*d));
//   int ret2=ioctl(fd,PUSH_DATA,&(*d));
//   int ret3=ioctl(fd,PUSH_DATA,&(*d));
//   int ret4=ioctl(fd,PUSH_DATA,&(*d));
//   int ret5=ioctl(fd,PUSH_DATA,&(*d));
//   int ret6=ioctl(fd,PUSH_DATA,&(*d));
//   int ret7=ioctl(fd,PUSH_DATA,&(*d));
//   int ret8=ioctl(fd,PUSH_DATA,&(*d));
//   int ret9=ioctl(fd,PUSH_DATA,&(*d));
//   int ret10=ioctl(fd,PUSH_DATA,&(*d));
  
  
  close(fd);
  
  free(d->_data);
  free(d);
 
 
	close(fd);
	return 0;
}
