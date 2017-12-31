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

extern uint64_t recordReads;
extern uint64_t recordWrites;
extern uint64_t indexReads;
extern uint64_t indexWrites;



/*TODO:
 * - interface
 * - create private functions for memory manager
 *	shoulb be done: redundant new node file writes :// (intead of wrtiting zeros just assign position, and update global position)
 */


int main() {
	BTree b(DEGREE);


	b.createTestFile();
	b.runTestFile();


	b.saveState();
	cout << "Record reads: " << recordReads << " Record writes: "<<recordWrites << endl;
	cout << "Index reads: " << indexReads << " Index writes: "<<indexWrites << endl;

	return 0;
}
