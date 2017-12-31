/*
 * TreeNode.cpp
 *
 *  Created on: Nov 26, 2017
 *  Author: Tomasz Rybicki
 */

#include "TreeNode.h"
#include "MemoryManager.h"
#include "BTree.h"

TreeNode::TreeNode()
	: m_treeRecords()
	, m_nodePointers()
	, m_parentPosition(0)
	, m_position(0)
{

}

TreeNode::~TreeNode() {
}

static int getDigits(rKey_t key){
	unsigned int number_of_digits = 0;

	do {
	     ++number_of_digits;
	     key /= 10;
	} while (key);

	return number_of_digits;
}

void TreeNode::print(bool vertical, int indent) {
	BTree::manager.updatePathToRoot(m_position);

	int numberWidth = 0;
	if (vertical) {
		cout << "\u2014";
	} else {
		for (int i = 0 - indent; i < indent * PRINT_WIDTH; i++){
			cout << " ";
		}
		cout << "\\";
	}

	cout << '[';

	bool first = true;
	for (auto it : m_treeRecords){
		if (first){
			cout << "\u2022|";
			first = false;
		}else{
			cout << "|\u2022|";
		}
		cout << it.key;
		numberWidth += getDigits(it.key);
	}
	cout << "|\u2022]";//<<m_position<<"("<<m_parentPosition<<")";

	int align = PRINT_WIDTH - (2 + 3*m_treeRecords.size() + 1 + numberWidth);
	for (int i = 0; i < align; i++){
		cout << " ";
	}

	/* Print children */
	if (!m_nodePointers.size()){
		cout << endl;
	}

	TreeNode* node = 0;
	vertical = true;
	for (auto child = m_nodePointers.rbegin();
			child != m_nodePointers.rend();
			child++) {

		if ((*child) == ERROR)
			continue;

		node = BTree::manager.getNode((*child));

		node->print(vertical, indent + 1);
		vertical = false;
	}

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

list<position_t> &TreeNode::getPointers() {
	return m_nodePointers;
}

list<treeRecord> &TreeNode::getTreeRecords() {
	return m_treeRecords;
}

void TreeNode::printIndex() {
	BTree::manager.updatePathToRoot(m_position);

	/* Print children */
	TreeNode* node = 0;
	auto record = m_treeRecords.begin();

	if (m_nodePointers.size()){

		for (auto child : m_nodePointers) {
			if (child == ERROR)
				continue;

			node = BTree::manager.getNode(child);
			node->printIndex();

			if (record != m_treeRecords.end()){
				cout << " Key: " << (*record).key << " \tposition: " << (*record).position << endl;
				record++;
			}

		}

	/* Leaf */
	} else {
		for (auto record : m_treeRecords){
			cout << " Key: " << record.key << " \tposition: " << record.position << endl;
		}
	}

	BTree::manager.updatePathToRoot(m_position);

}

void TreeNode::printRecords() {
	BTree::manager.updatePathToRoot(m_position);

	/* Print children */
	TreeNode* node = 0;
	Record* rec = 0;
	auto record = m_treeRecords.begin();

	if (m_nodePointers.size()){

		for (auto child : m_nodePointers) {

			if (child == ERROR)
				continue;

			node = BTree::manager.getNode(child);
			node->printRecords();

			if (record != m_treeRecords.end()){
				rec = BTree::manager.getRecord((*record).position, (*record).key);
						cout << " Key: " << rec->getID()
								<< " \tHeight: " << rec->getHeight()
								<< " \tRadius: " << rec->getRadius() << endl;
				record++;
			}
		}

	/* Leaf */
	} else {
		for (auto record : m_treeRecords){
			rec = BTree::manager.getRecord(record.position, record.key);

			if(!rec){
				if (DEBUG)
					cout << "NULL pointer!" << endl;
				continue;
			}

			cout << " Key: " << rec->getID()
					<< " \tHeight: " << rec->getHeight()
					<< " \tRadius: " << rec->getRadius() << endl;
		}
	}

	BTree::manager.updatePathToRoot(m_position);

}

bool TreeNode::operator ==(const TreeNode& other) const {
	if (m_parentPosition != other.m_parentPosition)
		return false;

	if (m_position != other.m_position)
		return false;

	if (m_nodePointers != other.m_nodePointers)
		return false;

	if (m_treeRecords != other.m_treeRecords)
		return false;

	return true;
}
