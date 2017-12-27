/*
 * TreeNode.cpp
 *
 *  Created on: Nov 26, 2017
 *  Author: Tomasz Rybicki
 */

#include "TreeNode.h"

TreeNode::TreeNode()
	: m_treeRecords()
	, m_nodePointers()
	, m_parentPosition(0)
{

}

TreeNode::~TreeNode() {
}

void TreeNode::print() {
	cout << "Node position: " << m_position << " Parent: " << m_parentPosition << endl;
	cout << '[';

	bool first = true;
	for (auto it : m_treeRecords){
		if (first){
			cout << "-|";
			first = false;
		}else{
			cout << "|-|";
		}
		cout << it.key;
	}
	cout << "|-]" << endl;
};

static bool comp(const treeRecord &a, const treeRecord &b)
{
	return a.key < b.key;
}

void TreeNode::insert(Record record, position_t position) {
	treeRecord tr = {record.getID(), position };
	m_treeRecords.push_back(tr);

	/* Sort the contents */
	m_treeRecords.sort(comp);

}


int TreeNode::countRecords() {
	return m_treeRecords.size();
}

list<position_t> TreeNode::getPointers() {
	return m_nodePointers;
}

list<treeRecord> TreeNode::getTreeRecords() {
	return m_treeRecords;
}
