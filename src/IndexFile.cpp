//============================================================================
// Name        : IndexFile.cpp
// Author      : Tomasz Rybicki
// Version     :
// Copyright   : 
// Description : File organization implemented with indexes utilizing B-trees
//============================================================================

#include <iostream>
#include "TreeNode.h"
#include "BTree.h"
#include "MemoryManager.h"

using namespace std;


/*TODO:
 * - load previous database state (position of free block)
 * - fisnish backend for records
 * - backend memory mangagment (deallocate old nodes when allocating new ones
 * - splitting and compensating
 * - interface
 *
 *	- crashes because m_freeBlock is not saved inbetween running app!!!!!!!!!!!!
 *
 */
int main() {

	{
	MemoryManager M(3);

	Record* rPtr1;
	rPtr1 = M.newRecord();
	rPtr1->setId(2);
	rPtr1->setHeight(2);
	rPtr1->setRadius(2);

	Record* rPtr2;
	rPtr2 = M.newRecord();
	rPtr2->setId(1);

	}
	MemoryManager M(3);
	M.getRecord(0, 2);
	M.getRecord(0, 1);

	M.printRecords();




	return 0;
}
