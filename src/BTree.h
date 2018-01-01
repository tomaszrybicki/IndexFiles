/*
 * BTree.h
 *
 *  Created on: Nov 26, 2017
 *  Author: Tomasz Rybicki
 */

#ifndef BTREE_H_
#define BTREE_H_

#include "TreeNode.h"
#include "Record.h"
#include "MemoryManager.h"



class BTree {
public:
	BTree(int degree);
	virtual ~BTree();

	/* Splits given node into two nodes, and reorganizes tree */
	void split(position_t nodePos);

	/* Inserts a record into index file */
	bool insert(Record record, position_t recordPos);

	bool insert(double h, double r, rKey_t key);

	bool insertWrapper(double h, double r, rKey_t key);

	/* Attempts compensation, returns true if possible and done */
	bool compensate(position_t node);

	/* Removed record with key */
	void removeRecord(rKey_t key);

	void removeRecordWrapper(rKey_t key);

	/* Pass 0 as any argument except key to not change its value */
	void updateRecord(rKey_t key, double h, double r, rKey_t newKey);

	/* Prints Btree structure */
	void print();

	/* Prints contents of BTree in sorted order */
	void printIndexes();

	/* Prints all stored records */
	void printRecords();

	/* Returns position of record with given key */
	position_t findKey(rKey_t key);

	Record* getRecord(rKey_t key);

	Record* getRecordWrapper(rKey_t key);

	/* The manager responsible for file access layer */
	static MemoryManager manager;

	/* Saves position of root node */
	void saveState();

	/* Test files are binary files where each operation is in following format:
	 * Offset
	 * 0x00 - type of operation (Update, Remove, Insert, Fetch, Print)
	 * 0x01 - key of the record
	 * 0x09 - new height (only for update and insert) - double format
	 * 0x11 - new radius (only for update and insert) - double format
	 */
	void runTestFile();

	void createTestFile();

	void interface();

	void printMenu();

	void interfaceInsert();

	void interfaceFetch();

	void interfaceUpdate();

	void interfaceRemove();

	/* Returns a node which contains or to which
	 *  a key should be inserted */
	position_t findLeafNodeForKey(unsigned long long key);

	position_t findNodeWithKey(unsigned long long key);

	void merge(position_t nodePos);

	void findNeighbourNodes(position_t nodePos, position_t &left, position_t &right);


private:
	/* The degree of B-tree */
	int m_degree;

	/* The root node of the tree */
	position_t m_root;

	/* Extended information printing */
	bool m_printInfo;
};

#endif /* BTREE_H_ */
