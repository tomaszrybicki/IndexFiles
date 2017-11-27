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
#include "NodeElements.h"
#include "Record.h"

using namespace std;

class TreeNode {
public:
	TreeNode();
	virtual ~TreeNode();

	/* Prints contents of the node */
	void print();

	list<NodeElement> getElements();

	/* Inserts record into index file and record file */
	void insert(Record record);


private:
	list<NodeElement> m_elements;
	unsigned long long m_parentPosition;
};

#endif /* TREENODE_H_ */
