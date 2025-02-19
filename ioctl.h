#ifndef IOCTL_TEST_H
#define IOCTL_TEST_H


#include "queue.h"





#define SET_SIZE _IOW('a', 'a', int)
#define PUSH_DATA _IOW('a', 'b', data_t )
#define POP_DATA  _IOW('a', 'c', data_t )

#endif
