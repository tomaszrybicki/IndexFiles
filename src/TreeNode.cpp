/*
 * TreeNode.cpp
 *
 *  Created on: Nov 26, 2017
 *      Author: lebowski
 */

#include "TreeNode.h"

TreeNode::TreeNode()
	: m_elements()
	, m_parentPosition(0)
{

}

TreeNode::~TreeNode() {
}

void TreeNode::print() {
	for (auto i : m_elements){
		cout << "Node location: " << i.getNode() << " Key: " << i.getKey() << endl;
	}
};


list<NodeElement> TreeNode::getElements() {
	return m_elements;
}


void TreeNode::insert(Record record) {
	NodeElement empty(0);

	/* If Node is empty, first add empty node pointer */
	if(m_elements.empty()){
		m_elements.push_back(empty);
	}

	/* Our record's index to be put in BTree */
	NodeElement index(0, record.getID());
	//register data

	m_elements.push_back(index);

	/* Add empty node pointer add the end */
	m_elements.push_back(empty);

	/* Sort the contents */
	sort();

}

/* TODO: fix performance, change sorting algorithm */
void TreeNode::sort() {
	/* Bubble sort omitting node pointers */

	if (m_elements.size() == 0 || m_elements.size() == 3){
		return;
	}

	list<NodeElement>::iterator current = m_elements.begin();
	current++;

	list<NodeElement>::iterator next = m_elements.begin();
	advance(next, 3);


	for (int i = 0; i < m_elements.size(); i++){
		for (int j = 0; j < m_elements.size(); j++){
			if ((*current).getKey() > next...)
				swap
		}
		//reset iterators
	}
}
