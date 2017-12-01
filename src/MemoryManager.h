/*
 *	MemoryManager.h
 *
 *  Created on: Nov 28, 2017
 *  Author:		Tomasz Rybicki
 */

#ifndef SRC_MEMORYMANAGER_H_
#define SRC_MEMORYMANAGER_H_

#include "TreeNode.h"
#include <iostream>
#include <fstream>
#include <map>

using namespace std;

/* MemoryManager is responsible for managing files.
 * Every Node or Record is identified by it's
 * position in file.
 *
 * If request is made for a Node or a Record
 * the manager returns it's memory address.
 * If it wasn't in memory already, it is loaded
 *
 * Node entries in file are fixed width
 */
class MemoryManager {
public:
	MemoryManager(int degree);
	virtual ~MemoryManager();

	/* Returns pointer to allocated node */
	TreeNode* getNode(position_t nodePosition);

	/* Saves node in file and frees allocated memory */
	void deallocateNode(TreeNode* node);

	/* Saves node in file and keeps allocated memory */
	void updateNode(TreeNode* node);

	/* Creates a new node in file,
	 * loads it into memory and returns it's address */
	TreeNode* newNode();

	/* Saves all allocated nodes to file */
	void syncNodes();


	/* Creates a new record in file, loads it into memory
	 * and returns the pointer */
	Record* newRecord();

	/* Loads the block position into memory
	 * and returns the pointer */
	Record* getRecord(position_t position, rKey_t key);

	/* Loads given block of given position into memory */
	void getBlock(position_t position);

	//deaalocate - set header!




private:
	/* File holding all the trees nodes */
	string m_nodesFile;

	/* File holding all the records */
	string m_recordsFile;

	/* Degree of B-Tree */
	int m_degree;

	/* Size in bytes of each nodes entry in file */
	int m_nodeEntrySize;

	/* Map of nodes loaded in memory */
	map<position_t, TreeNode*> m_allocatedNodes;

	/* Map of record pages loaded in memory */
	map<position_t, list<Record*>> m_allocatedRecords;

	/* Position of not full block */
	position_t m_freeBlock;

};

#endif /* SRC_MEMORYMANAGER_H_ */
