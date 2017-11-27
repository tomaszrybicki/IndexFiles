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

bool comparisonNode(const NodeElement e1, const NodeElement e2){
	if
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
	m_elements.sort();

}
