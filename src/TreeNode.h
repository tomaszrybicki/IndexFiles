/*
 * TreeNode.h
 *
 *  Created on: Nov 26, 2017
 *  Author: Tomasz Rybicki
 */

#ifndef TREENODE_H_
#define TREENODE_H_

#include <list>
#include <iostream>
#include "Defines.h"
#include "Record.h"

using namespace std;

class TreeNode {
public:
	friend class MemoryManager;

	TreeNode();
	virtual ~TreeNode();

	/* Prints contents of the node */
	void print();

	list<position_t> getPointers();

	list<treeRecord> getTreeRecords();

	/* Inserts record into index file and record file
	 * Node is then sorted */
	void insert(Record record);

	/* Returns the number of stored records in the node */
	int countRecords();

	position_t getParentPosition() const {
		return m_parentPosition;
	}

	position_t getPosition() const {
		return m_position;
	}

private:
	/* Sorts the node's element by key
	 * Leaves node pointers in original place */
	void sort();

private:
	list<treeRecord> m_treeRecords;
	list<position_t> m_nodePointers;
	position_t m_parentPosition;
	position_t m_position;
};

#endif /* TREENODE_H_ */
