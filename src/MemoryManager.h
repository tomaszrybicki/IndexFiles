/*
 *	MemoryManager.h
 *
 *  Created on: Nov 28, 2017
 *  Author:		Tomasz Rybicki
 */

#ifndef SRC_MEMORYMANAGER_H_
#define SRC_MEMORYMANAGER_H_

#include "TreeNode.h"

/* MemoryManager is responsible for managing files.
 * Every Node or Record is identified by it's
 * position in file.
 *
 * If request is made for a Node or a Record
 * the manager returns it's memory address.
 * If it wasn't in memory already, it is loaded
 */
class MemoryManager {
public:
	MemoryManager();
	virtual ~MemoryManager();

	TreeNode* getNode(unsigned long long nodePosition);

private:
	/* File holding all the trees nodes */
	//NodesFile

	/* File holding all the records */
	//RecordsFile
};

#endif /* SRC_MEMORYMANAGER_H_ */
