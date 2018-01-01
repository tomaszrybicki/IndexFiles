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

	/* Prints contents of the node and its children
	 * vertical sets output for first child
	 *
	 * Aligning works for 2 digit keys properly
	 * change PRINT_WIDTH to increase */
	void print(bool vertical, int indent);

	void printIndex();

	void printRecords();

	list<position_t> &getPointers();

	list<treeRecord> &getTreeRecords();

	/* Inserts record into index file and record file
	 * Node is then sorted */
	void insert(Record record, position_t position);

	/* Returns the number of stored records in the node */
	int countRecords();

	position_t getParentPosition() const {
		return m_parentPosition;
	}

	position_t getPosition() const {
		return m_position;
	}

	bool operator==(const TreeNode& other) const;


	list<treeRecord> m_treeRecords;
	list<position_t> m_nodePointers;
	position_t m_parentPosition;
	position_t m_position;
};

#endif /* TREENODE_H_ */
