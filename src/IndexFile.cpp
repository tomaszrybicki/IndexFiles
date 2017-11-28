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

using namespace std;


/*TODO:
 *
 * - splitting and compensating
 * - file backend with buffers (FileManager)
 * - interface
 *
 */
int main() {

	BTree B(3);

	Record r1(1,1,5);
	Record r2(2,1,2);
	Record r3(1,1,41);
	Record r4(1,1,5);

	B.insert(r1);
	B.insert(r2);
	B.insert(r3);
	B.insert(r4);

	B.print();

	return 0;
}
