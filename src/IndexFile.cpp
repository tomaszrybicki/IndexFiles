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
 * - splitting and compensating
 * - interface
 * - create private functions for memory manager
 *
 *
 *	shoulb be done: redundant new node file writes :// (intead of wrtiting zeros just assign position, and update global position)
 *	Fancy:
 *
 */
int main() {

	{
	MemoryManager M(3,1);
	Record* rPtr1;

	for (int i =1; i < 6; i++){
		rPtr1 = M.newRecord();
		rPtr1->setId(i);
	}

	M.printRecords();
	}

	MemoryManager M(3,1);
	M.getRecord(0, 1);
	M.deleteRecord(0,1);
	Record* r = M.newRecord();
	r->setId(15);
	M.printRecords();

	return 0;
}
