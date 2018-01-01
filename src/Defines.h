#ifndef DEFINES_H_
#define DEFINES_H_

#include <iostream>

#define ERROR (-1)
#define NO_CHILDREN 0
#define WRONG_ELEMENT 0

#define MAX_BUFFERED_HEIGHT 5
#define RECORDS_PER_PAGE 2
#define RECORD_PAGE_SIZE (((sizeof(rKey_t) + sizeof(double) + sizeof(double)) * RECORDS_PER_PAGE) + 1)
#define STATE_FILE_NAME "State.bin"
#define TEST_FILE_NAME "Test.bin"
#define KiB 1024

#define DEBUG 0
#define DEGREE 1
#define BUFFER_SIZE 10
#define DIGIT_ALIGN 2
#define	PRINT_WIDTH (DIGIT_ALIGN*2*DEGREE + 2*DEGREE + 1 + 4*DEGREE + 2 + 1 + 9*DEBUG)
#define MAX_RADIUS 10
#define MAX_HEIGHT 10
#define MIN_RADIUS 1
#define MIN_HEIGHT 1


typedef std::streamoff position_t;
typedef unsigned long long rKey_t;
typedef unsigned long long sequenceID_t;

struct treeRecord{
	rKey_t key;
	position_t position;

	bool operator==(const treeRecord& y) const {
	    if (key != y.key)
	    	return false;

	    if (position != y.position)
	    	return false;

	    return true;
	}
};



#endif
