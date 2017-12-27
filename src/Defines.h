#ifndef DEFINES_H_
#define DEFINES_H_

#include <iostream>

typedef std::streampos position_t;
typedef unsigned long long rKey_t;
typedef unsigned long long sequenceID_t;

typedef struct{
	rKey_t key;
	position_t position;
}treeRecord;


#define NO_CHILDREN 0
#define WRONG_ELEMENT 0

#define MAX_BUFFERED_HEIGHT 5
#define RECORDS_PER_PAGE 2
#define RECORD_PAGE_SIZE (((sizeof(rKey_t) + sizeof(double) + sizeof(double)) * RECORDS_PER_PAGE) + 1)
#define STATE_FILE_NAME "State.bin"

#define KiB 1024

#define MAX_RADIUS 10
#define MAX_HEIGHT 10
#define MIN_RADIUS 1
#define MIN_HEIGHT 1

#endif
