//reader.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>


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
  
  data_t d;
  ioctl(fd,PRINT,d);
 
 
	close(fd);
	return 0;
}
