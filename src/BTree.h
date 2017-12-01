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



class BTree {
public:
	BTree(int degree);
	virtual ~BTree();

	/* Splits given node into two nodes, and reorganizes tree */
	void split(TreeNode* node);

	/* Inserts a record into data file and index file */
	void insert(Record record);

	/* Attempts compensation, returns true if possible */
	bool compensate(unsigned long long node);

	void print();

private:
	/* Returns a node to which a key should be inserted */
	TreeNode* findLeafNodeForKey(unsigned long long key);

private:
	/* The degree of B-tree */
	int m_degree;

	/* The root node of the tree */
	position_t m_root;
};

#endif /* BTREE_H_ */
