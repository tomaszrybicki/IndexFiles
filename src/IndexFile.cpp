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




int main() {
	BTree b(DEGREE);
	b.createTestFile();

	b.interface();

	b.saveState();
	return 0;
}
