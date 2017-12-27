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
 * along with its whole page(block)
 *
 * Each map of allocated nodes/pages has
 * a complementary map of sequence id of each element
 * (incremented globally with each call to element)
 *
 * Node entries in file are fixed width
 */
class MemoryManager {
public:
	MemoryManager(int degree, int limit);
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

	/* Creates a new record in file, loads it into memory
	 * and returns the pointer, position is filled with record position
	 * Caller has to set id for the record before it's deallocated */
	Record* newRecord(position_t* position);


	/* Updates file to memory contents */
	void syncRecords();

	/* Loads the block position into memory
	 * and returns the pointer or NULL if doesn't exist */
	Record* getRecord(position_t position, rKey_t key);

	/* Loads into memory block with given position,
	 * position must be a valid position of block */
	void getBlock(position_t position);

	/* Saves block to file and frees memory */
	void deallocateBlock(position_t position);

	/* Prints contents of record file as well as
	 * contents of page buffers */
	void printRecords();

	/* Save object (databases) state such as pointer to free block */
	void saveState();

	/* Load state from previous application run */
	void loadState();

	/* Deletes record from page */
	void deleteRecord(position_t position, rKey_t key);

	/* Updates statistics about Node usage, increments globalSid */
	void updateNodeStats(position_t position);

	/* Updates statistics about page usage, increments globalSid */
	void updatePageStats(position_t position);

	/* Checks if there isnt too many nodes or records in memory and
	 * removes the ones last used from memory if needed
	 */
	void maintance();


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

	/* Map of sequence ids of nodes loaded in memory */
	map<position_t, sequenceID_t> m_allocatedNodesSid;

	/* Map of record pages loaded in memory */
	map<position_t, list<Record*>> m_allocatedRecords;

	/* Map of record pages loaded in memory sequence ids*/
	map<position_t, sequenceID_t> m_allocatedRecordsSid;

	/* Position of not full block */
	position_t m_freeBlock;

	/* Number of pages or nodes which can be held in memory */
	int m_memoryLimit;

	/* Number of pages held in memory */
	int m_pagesInMemory;

	/* Number of trees nodes held in memory */
	int m_nodesInMemory;

	sequenceID_t m_globalSid;
};

#endif /* SRC_MEMORYMANAGER_H_ */
