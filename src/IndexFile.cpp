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
 * - fisnish backend for records
 * - backend memory mangagment (deallocate old nodes when allocating new ones
 * - splitting and compensating
 * - interface
 *
 */
int main() {

	MemoryManager M(30);

	TreeNode* A = new TreeNode;
	Record r1(1, 1, 10);
	Record r2(1, 1, 2);
	A->insert(r1);
	A->insert(r2);
	A->print();

	M.deallocateNode(A);
	A = M.getNode(0);
	A->print();


	return 0;
}
