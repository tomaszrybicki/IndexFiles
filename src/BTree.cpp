/*
 * BTree.cpp
 *
 *  Created on: Nov 26, 2017
 *  Author: Tomasz Rybicki
 */

#include "BTree.h"

BTree::BTree(int degree)
	:m_degree(degree)
{
	m_root = new TreeNode();
}

BTree::~BTree() {
}

void BTree::insert(Record record) {
	/* Check if exists */
	// ...

	/* Find a node which should hold value of our key */
	TreeNode* node = findLeafNodeForKey(record.getID());
	node->insert(record);

	/* Check node number of elements and split or smh else accordingly */
}

void BTree::print() {
	m_root->print();
}

TreeNode* BTree::findLeafNodeForKey(unsigned long long key) {
	NodeElement* prevElement = NULL;
	TreeNode* currentNode = m_root;

	/* Get root node elements */
	list<NodeElement> elements = currentNode->getElements();
	list<NodeElement>::iterator it = elements.begin();

	if(elements.empty()){
		return currentNode;
	}

	prevElement = &(*it);
	it++;

	/* Look for key bigger than ours */
	for (unsigned int i = 0; i < elements.size(); i++){

		/* End if we are in leaf */
		if (prevElement->getNode() == NO_CHILDREN){
			return currentNode;
		}

		/* Change node if we found the proper child node */
		if (key <= (*it).getKey()){
			currentNode = prevElement->getNode();
			elements = currentNode->getElements();
			it = elements.begin();
			prevElement = &(*it);
			it++;

			/* Reset loop */
			i = -1;
			continue;
		}
		it++;
		it++;

		/* If end of node */
		if (it == elements.end()){
			prevElement = &(*(--it));
			currentNode = prevElement->getNode();
			elements = currentNode->getElements();
			it = elements.begin();
			prevElement = &(*it);
			it++;

			/* Reset loop */
			i = -1;
			continue;
		}
	}

	cout << "Should not end here! (i guess)" << endl;
	return currentNode;
}
