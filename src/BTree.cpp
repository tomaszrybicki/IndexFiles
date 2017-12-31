/*
 * BTree.cpp
 *
 *  Created on: Nov 26, 2017
 *  Author: Tomasz Rybicki
 */

#include "BTree.h"
#include "MemoryManager.h"
#include <algorithm>
#include <fstream>
#include <stack>

using namespace std;

MemoryManager BTree::manager(DEGREE, BUFFER_SIZE);

BTree::BTree(int degree)
	: m_degree(degree)
{
	position_t rootPos = manager.getRootNodePosition();

	if (rootPos == ERROR){
		cout << "No previous BTree state found." << endl;
		TreeNode* node = manager.newNode();
		m_root = node->getPosition();
	} else {
		m_root = rootPos;
	}
}

BTree::~BTree() {
}

static bool compareRecord(const treeRecord &a, const treeRecord &b)
{
	return a.key < b.key;
}

static bool compareNode(const position_t &a, const position_t &b)
{
	if (BTree::manager.getNode(a)->m_treeRecords.begin() ==
			BTree::manager.getNode(a)->m_treeRecords.end()
			|| BTree::manager.getNode(b)->m_treeRecords.begin() ==
					BTree::manager.getNode(b)->m_treeRecords.end()){
		return false;
	}
	rKey_t aFirstKey = BTree::manager.getNode(a)->m_treeRecords.front().key;
	rKey_t bFirstKey = BTree::manager.getNode(b)->m_treeRecords.front().key;

	return aFirstKey < bFirstKey;
}


bool BTree::insert(Record record, position_t recordPos) {
	/* Check if exists */
	if (findKey(record.getID()) != ERROR){
		cout << "Error inserting record: key already exists" << endl;
		return false;
	}

	if (record.getID() <= 0){
		cout << "Invalid key!" << endl;
		return false;
	}

	/* Find node for key */
	position_t leafPos = findLeafNodeForKey(record.getID());
	TreeNode* node = manager.getNode(leafPos);

	/* There is a place for record */
	if (node->countRecords() < 2 * m_degree){
		node->insert(record, recordPos);
		return true;
	}

	/* There is no place, attempt compensation */
	else {
		/* Insert (overfill) */
		node->insert(record, recordPos);
		bool result = compensate(leafPos);

		if (result) {
			return true;
		} else {
			split(leafPos);
		}

	}
	return true;
}

void BTree::print() {
	cout << "BTree structure: " << endl;
	TreeNode* node = manager.getNode(m_root);
	node->print(true, 0);
	cout << endl;
}

void BTree::split(position_t nodePos) {
	/* Get node for splitting */
	TreeNode* splitted = manager.getNode(nodePos);
	TreeNode* parent;

	/* Get parent node */
	if (nodePos == m_root){
		parent = BTree::manager.newNode();
		parent->m_parentPosition = ERROR;
		parent->m_nodePointers.push_back(m_root);
		m_root = parent->m_position;
		splitted->m_parentPosition = parent->m_position;

	} else {
		parent = manager.getNode(splitted->m_parentPosition);
	}

	/* Get new node */
	TreeNode* newNode = 0;
	newNode = BTree::manager.newNode();
	newNode->m_parentPosition = splitted->m_parentPosition;


	/* Redistribute */
	list<treeRecord> recordBuffer, firstPage, secondPage;
	list <position_t> ptrBuffer, firstPtrs, secondPtrs;
	treeRecord parentElement;
	recordBuffer.insert(recordBuffer.end(), splitted->m_treeRecords.begin(),
			splitted->m_treeRecords.end());
	ptrBuffer.insert(ptrBuffer.end(), splitted->m_nodePointers.begin(),
			splitted->m_nodePointers.end());
	recordBuffer.sort(compareRecord);
	ptrBuffer.sort(compareNode);

	/* Redistribute records */
	uint32_t i = 0;
	for (auto it : recordBuffer){
		/* Write to first page */
		if (i < recordBuffer.size()/2){
			firstPage.push_back(it);

		/* Write to parent */
		} else if (i == recordBuffer.size()/2) {
			parentElement = it;

		/* Write to second page */
		} else {
			secondPage.push_back(it);
		}

		i++;
	}

	/* Redistribute pointers */
	i = 0;
	for (auto it = ptrBuffer.begin(); it != ptrBuffer.end(); it++) {
		/* Write to first page */
		if (i <= recordBuffer.size()/2){
			firstPtrs.push_back((*it));

		} else {
			/* Update moved pointers parents */
			manager.getNode(*it)->m_parentPosition = newNode->m_position;
			secondPtrs.push_back((*it));
		}
		i++;
	}


	newNode->m_treeRecords = secondPage;
	newNode->m_nodePointers = secondPtrs;
	splitted->m_treeRecords = firstPage;
	splitted->m_nodePointers = firstPtrs;

	/* Update parent */
	parent->m_treeRecords.push_back(parentElement);
	parent->m_nodePointers.push_back(newNode->m_position);
	parent->m_treeRecords.sort(compareRecord);
	parent->m_nodePointers.sort(compareNode);

	/* Parent overflow */
	if (parent->countRecords() > 2 * DEGREE){

		/* Attempt compensation */
		if (!compensate(parent->m_position)){

			/* Split parent if necessary */
			split(parent->m_position);
		}
	}


}


bool BTree::compensate(position_t nodePos) {
	TreeNode* compensated = manager.getNode(nodePos);
	position_t parentPos = compensated->getParentPosition();

	if(parentPos == ERROR){
		return false;
	}
	TreeNode* parent = manager.getNode(parentPos);

	/* Find neighbours  */
	position_t lastPos = parent->getPointers().front();
	position_t neighPos1 = ERROR, neighPos2 = ERROR;
	TreeNode* neigh1;
	TreeNode* neigh2;
	treeRecord middle, middle1, middle2;
	bool nextIsRightNeigbour = false;

	int i = 0;
	for (auto pos : parent->getPointers()){
		/* Save rightmost neighbour position */
		if (nextIsRightNeigbour) {
			neighPos2 = pos;

			/* Save middle tree record */
			auto it = parent->getTreeRecords().begin();
			advance(it, i-1);
			middle2 = *it;
			break;
		}

		if (pos == compensated->getPosition()){
			/* Save leftmost neighbour if exists */
			if (pos != lastPos) {
				neighPos1 = lastPos;

				/* Save middle tree record  */
				list<treeRecord>::iterator it = parent->getTreeRecords().begin();
				advance(it, i-1);
				middle1 = (*it);
			}
			nextIsRightNeigbour = true;
		}
		lastPos = pos;
		i++;
	}

	/* See if any of neighbouring nodes are not full */
	if (neighPos1 != ERROR)
		neigh1 = manager.getNode(neighPos1);
	if (neighPos2 != ERROR)
		neigh2 = manager.getNode(neighPos2);
	position_t compNodePos;
	bool possible = false;

	if (neighPos1 != ERROR) {
		int totalRecords = neigh1->countRecords() + compensated->countRecords();
		if (totalRecords <= 2 * 2 * m_degree && totalRecords >= 2 * m_degree){
			compNodePos = neighPos1;
			middle = middle1;
			possible = true;
		}
	}

	if (neighPos2 != ERROR && !possible){
		int totalRecords = neigh2->countRecords() + compensated->countRecords();
		if (totalRecords <= 2 * 2 * m_degree && totalRecords >= 2 * m_degree){
			compNodePos = neighPos2;
			middle = middle2;
			possible = true;
		}
	}

	/* Compensation is not possible */
	if (!possible) {
		return false;
	}


	/* Actual compensation */
	list<treeRecord> recordBuffer;
	list<position_t> ptrBuffer;
	compensated = manager.getNode(nodePos);
	TreeNode* compNode = manager.getNode(compNodePos);

	/* Return false when compensation is not possible for underflowed node */
	if (compensated->countRecords() < m_degree && compNode->countRecords() <= m_degree){
		return false;
	}

	/* Add overfilled node, its neighbour and one parent key to common buffer */
	recordBuffer.insert(recordBuffer.begin(), compensated->m_treeRecords.begin(),
			compensated->m_treeRecords.end());
	ptrBuffer.insert(ptrBuffer.begin(), compensated->m_nodePointers.begin(),
			compensated->m_nodePointers.end());

	recordBuffer.insert(recordBuffer.begin(), compNode->m_treeRecords.begin(),
			compNode->m_treeRecords.end());
	ptrBuffer.insert(ptrBuffer.begin(), compNode->m_nodePointers.begin(),
				compNode->m_nodePointers.end());

	recordBuffer.push_back(middle);

	/* Sort them */
	recordBuffer.sort(compareRecord);
	ptrBuffer.sort(compareNode);

	/* Redistribute them - two pages and one record for parent */
	int elementCount = recordBuffer.size();
	list <treeRecord> firstPage, secondPage;
	list <position_t> firstPtrs, secondPtrs;
	treeRecord parentElement;
	i = 0;

	for (auto it = recordBuffer.begin(); it != recordBuffer.end(); it++) {
		/* Write to first page */
		if (i < elementCount/2){
			firstPage.push_back((*it));

		/* Write to parent */
		} else if (i == elementCount/2) {
			parentElement = (*it);

		/* Write to second page */
		} else {
			secondPage.push_back((*it));
		}
		i++;
	}

	i = 0;
	for (auto it = ptrBuffer.begin(); it != ptrBuffer.end(); it++) {
		/* Write to first page */
		if (i <= elementCount/2){
			firstPtrs.push_back((*it));

		} else {
			secondPtrs.push_back((*it));
		}
		i++;
	}

	/* If our neigbhour is to the left */
	if (middle == middle1){
		compNode->m_treeRecords = firstPage;
		compNode->m_nodePointers = firstPtrs;
		compensated->m_treeRecords = secondPage;
		compensated->m_nodePointers = secondPtrs;
	} else {
		compNode->m_treeRecords = secondPage;
		compNode->m_nodePointers = firstPtrs;
		compensated->m_treeRecords = firstPage;
		compensated->m_nodePointers = secondPtrs;
	}

	for (auto &j : parent->m_treeRecords){
		if (j == middle){
			j = parentElement;
		}
	}

	return true;
}

position_t BTree::findKey(rKey_t key) {
	/* If no root - end */
	if (m_root == ERROR) {
		return ERROR;
	}

	uint32_t i = 0;
	TreeNode* node;
	node = manager.getNode(m_root);
	list<treeRecord> records = node->getTreeRecords();
	list<position_t> pointers = node->getPointers();


	while (node != NULL) {
		/* Look for key in nodes records */
		for (auto it : records) {

			/* Found record - return its position */
			if (it.key == key){
				return it.position;
			}
		}

		/* Break when leaf and not found yet */
		if (pointers.size() == 0){
			break;
		}

		/* Sought key is smaller than this nodes smallest key */
		else if (key < records.front().key) {
			node = manager.getNode(pointers.front());
			records = node->getTreeRecords();
			pointers = node->getPointers();
		}

		/* Sought key is bigger than this nodes biggest key */
		else if (key > records.back().key) {
			node = manager.getNode(pointers.back());
			records = node->getTreeRecords();
			pointers = node->getPointers();
		}

		/* Choose proper node (.. node X [node] Y node .. where X < key < Y ) */
		else {
			auto nodeIt = pointers.begin();
			i = 0;

			for (auto it : records) {

				/* Found proper node index (i)*/
				if (it.key > key){

					/* Get iterator to the proper node */
					advance(nodeIt, i);

					node = manager.getNode((*nodeIt));
					records = node->getTreeRecords();
					pointers = node->getPointers();

					break;
				}

				i++;
			}
		}
	}

	return ERROR;
}

bool BTree::insert(double h, double r, rKey_t key) {
	/* Check if key exists */
	if (findKey(key) != ERROR){
		cout << "Can't insert record: key already exists!" << endl;
		return false;
	}

	position_t pos;
	Record* newRecord = BTree::manager.newRecord(&pos);

	newRecord->setId(key);

	if (insert(*newRecord, pos)) {
		newRecord->setHeight(h);
		newRecord->setRadius(r);
		return true;

	} else {
		return false;
	}

}

Record* BTree::getRecord(rKey_t key) {
	position_t pos = findKey(key);
	Record* rec = manager.getRecord(pos, key);

	return rec;
}

void BTree::printIndexes() {
	TreeNode* node = manager.getNode(m_root);
	cout << "Index content: " << endl;
	node->printIndex();
	cout << endl;
}

void BTree::printRecords() {
	TreeNode* node = manager.getNode(m_root);
	cout << "Records: " << endl;
	node->printRecords();
	cout << endl;
}

void BTree::removeRecord(rKey_t key) {
	/* Find node with key */
	position_t updatedNodePos = findNodeWithKey(key);
	if (updatedNodePos == ERROR){
		cout << "Can't remove nonexistent record!" << endl;
		return;
	}

	TreeNode*  updatedNode = manager.getNode(updatedNodePos);
	treeRecord toRemove;
	position_t leftPtr;

	/* If node is leaf, remove and finish */
	if (updatedNode->m_nodePointers.empty()){
		for (auto record : updatedNode->m_treeRecords){
			if (record.key == key){
				toRemove = record;
				break;
			}
		}
		updatedNode->m_treeRecords.remove(toRemove);

		/* See if leaf still has at least m_degree children */
		TreeNode* underflowed = updatedNode;
		position_t parentPos;

		while (underflowed->countRecords() < m_degree){

			/* Root underflow */
			if (underflowed->m_parentPosition == ERROR){
				if (underflowed->countRecords() == 0){
					position_t childPos = underflowed->m_nodePointers.front();
					TreeNode* child = manager.getNode(childPos);
					child->m_parentPosition = ERROR;
					underflowed->m_nodePointers.clear();
					m_root = childPos;
				}

				break;
			}

			/* Attempt compensation */
			if (!compensate(underflowed->m_position)){

				parentPos = underflowed->m_parentPosition;
				merge(underflowed->m_position);

				/* Check for underflow in parent */
				underflowed = manager.getNode(parentPos);
			}
		}

	/* Else find biggest key smaller than removed key */
	} else {
		auto it = updatedNode->m_nodePointers.begin();
		int i = 0;
		for (auto record : updatedNode->m_treeRecords){

			/* Found pointer to the node left of removed key */
			if (record.key == key){
				advance(it, i);
				leftPtr = (*it);
				break;
			}

			i++;
		}

		TreeNode* child = manager.getNode(leftPtr);
		position_t nextChild;
		Record* removed;


		/* Go down to the leaf with biggest key */
		while (!child->m_nodePointers.empty()){
			nextChild = child->m_nodePointers.back();
			child = manager.getNode(nextChild);
		}

		/* Copy and remove the record to be inserted in place of removed one */
		treeRecord biggest = child->m_treeRecords.back();
		child->m_treeRecords.remove(biggest);

		/* Remove the node to be removed and insert the biggest node */
		for (auto &record : updatedNode->m_treeRecords){
			removed = manager.getRecord(record.position, record.key);
			record.key = biggest.key;
			record.position = biggest.position;
		}

		/* See if leaf still has at least m_degree children */
		TreeNode* underflowed = child;
		while (underflowed->countRecords() < m_degree && underflowed->m_parentPosition != ERROR){

			/* Attempt compensation */
			if (!compensate(underflowed->m_position)){
				merge(underflowed->m_position);

				/* Check for underflow in parent */
				underflowed = manager.getNode(underflowed->m_parentPosition);
			}
		}

		/* Remove the node from actual records file (set id == 0) */
		removed->setHeight(0);
		removed->setId(0);
		removed->setRadius(0);
	}

}

position_t BTree::findLeafNodeForKey(rKey_t key) {
	/* If no root - end */
	if (m_root == ERROR) {
		return ERROR;
	}

	uint32_t i = 0;
	TreeNode* node;
	node = manager.getNode(m_root);
	list<treeRecord> records = node->getTreeRecords();
	list<position_t> pointers = node->getPointers();


	while (node != NULL) {
		/* Break when leaf */
		if (pointers.size() == 0){
			return node->getPosition();
		}

		/* Sought key is smaller than this nodes smallest key */
		else if (key < records.front().key) {
			node = manager.getNode(pointers.front());
			records = node->getTreeRecords();
			pointers = node->getPointers();
		}

		/* Sought key is bigger than this nodes biggest key */
		else if (key > records.back().key) {
			node = manager.getNode(pointers.back());
			records = node->getTreeRecords();
			pointers = node->getPointers();
		}

		/* Choose proper node (.. node X [node] Y node .. where X < key < Y ) */
		else {
			auto nodeIt = pointers.begin();
			i = 0;

			for (auto it : records) {

				/* Found proper node index (i)*/
				if (it.key > key){

					/* Get iterator to the proper node */
					advance(nodeIt, i);

					node = manager.getNode((*nodeIt));
					records = node->getTreeRecords();
					pointers = node->getPointers();

					break;
				}

				i++;
			}
		}
	}

	return ERROR;
}

void BTree::saveState() {
	manager.setRootNodePosition(m_root);
}

position_t BTree::findNodeWithKey(unsigned long long key) {
	/* If no root - end */
	if (m_root == ERROR) {
		return ERROR;
	}

	uint32_t i = 0;
	treeRecord toFind = {key, 0};
	TreeNode* node;
	node = manager.getNode(m_root);
	list<treeRecord> records = node->getTreeRecords();
	list<position_t> pointers = node->getPointers();


	while (node != NULL) {
		/* Look for key in nodes records */
		auto iter = find(records.begin(), records.end(), toFind);

		/* Found record - return its position */
		if (iter != records.end()){
			return node->m_position;
		}

		/* Break when leaf and not found yet */
		if (pointers.size() == 0){
			break;
		}

		/* Sought key is smaller than this nodes smallest key */
		else if (key < records.front().key) {
			node = manager.getNode(pointers.front());
			records = node->getTreeRecords();
			pointers = node->getPointers();
		}

		/* Sought key is bigger than this nodes biggest key */
		else if (key > records.back().key) {
			node = manager.getNode(pointers.back());
			records = node->getTreeRecords();
			pointers = node->getPointers();
		}

		/* Choose proper node (.. node X [node] Y node .. where X < key < Y ) */
		else {
			auto nodeIt = pointers.begin();
			i = 0;

			for (auto it : records) {

				/* Found proper node index (i)*/
				if (it.key > key){

					/* Get iterator to the proper node */
					advance(nodeIt, i);

					node = manager.getNode((*nodeIt));
					records = node->getTreeRecords();
					pointers = node->getPointers();

					break;
				}

				i++;
			}
		}
	}

	return ERROR;
}

void BTree::merge(position_t nodePos) {
	TreeNode* underflowed = manager.getNode(nodePos);
	list<treeRecord> recordBuffer;
	list<position_t> ptrBuffer;
	position_t left, right;

	findNeighbourNodes(nodePos, left, right);
	position_t resultNodePos = (left == ERROR) ? right : left;
	TreeNode* resultNode = manager.getNode(resultNodePos);

	if (resultNode->m_position == ERROR || underflowed->m_parentPosition == ERROR){
		cout << "Trying to merge root!" << endl;
		return;
	}

	/* Set children of deleted node to point to new parent */
	for (auto it : underflowed->m_nodePointers){
		manager.getNode(it)->m_parentPosition = resultNodePos;
	}

	underflowed = manager.getNode(nodePos);

	recordBuffer.insert(recordBuffer.end(),
			underflowed->m_treeRecords.begin(),
			underflowed->m_treeRecords.end());
	ptrBuffer.insert(ptrBuffer.end(),
			underflowed->m_nodePointers.begin(),
			underflowed->m_nodePointers.end());

	recordBuffer.insert(recordBuffer.end(),
			resultNode->m_treeRecords.begin(),
			resultNode->m_treeRecords.end());
	ptrBuffer.insert(ptrBuffer.end(),
			resultNode->m_nodePointers.begin(),
			resultNode->m_nodePointers.end());

	/* Get record from parent and remove it along with pointer to removed node */
	TreeNode* parent = manager.getNode(underflowed->m_parentPosition);
	treeRecord record;

	auto leftPtr = parent->m_nodePointers.begin();
	for (auto rec : parent->m_treeRecords){

		if ((*leftPtr) == nodePos || (*leftPtr) == resultNodePos){
			/* Middle record to be put in merged node */
			record = rec;
			break;
		}
		leftPtr++;
	}

	/* Remove from parent */
	parent->m_nodePointers.remove(nodePos);
	parent->m_treeRecords.remove(record);

	/* Remove old node */
	underflowed->m_nodePointers.clear();
	underflowed->m_treeRecords.clear();
	underflowed->m_parentPosition = 0;
	underflowed->m_position = 0;

	/* Add all to merged node */
	recordBuffer.push_back(record);

	resultNode->m_nodePointers = ptrBuffer;
	resultNode->m_treeRecords = recordBuffer;

	resultNode->m_nodePointers.sort(compareNode);
	resultNode->m_treeRecords.sort(compareRecord);
}

void BTree::updateRecord(rKey_t key, double h, double r, rKey_t newKey) {
	if (newKey == 0){
		Record* record = getRecord(key);

		if (h){
			record->setHeight(h);
		}

		if (r){
			record->setRadius(r);
		}

		return;

	/* Change records key */
	}else{
		position_t pos = findKey(key);
		Record* record = manager.getRecord(pos, key);

		removeRecord(key);

		if (h){
			record->setHeight(h);
		}

		if (r){
			record->setRadius(r);
		}

		record->setId(newKey);

		insert(*record, pos);
	}
}

void BTree::interface() {
}

void BTree::runTestFile() {
	ifstream file(TEST_FILE_NAME, ios::in|ios::binary);

	/* File cannot be opened */
	if (!file.good()){
		cerr << "File cannot be opened: " << TEST_FILE_NAME << endl;
		return;
	}

	file.seekg(0, ios::end);
	position_t end = (int) file.tellg();
	file.seekg(0,ios::beg);

	char type;
	rKey_t key = 0;
	rKey_t newKey = 0;
	double height, radius;

	while(file.tellg() < end){
		file.read(&type, 1);
		file.read((char*)&key, 8);
		file.read((char*)&height, 8);
		file.read((char*)&radius, 8);
		file.read((char*)&newKey, 8);


		switch (type){
		case 'I':
			cout << "inserting key: " << key << endl;
			insert(height, radius, key);
			break;

		case 'U':
			updateRecord(key, height, radius, newKey);
			break;

		case 'P':
			print();
			printIndexes();
			printRecords();
			break;

		}

	}

}

void BTree::createTestFile() {
	ofstream file(TEST_FILE_NAME, ios::out|ios::binary);

	/* File cannot be opened */
	if (!file.good()){
		cerr << "File cannot be opened: " << TEST_FILE_NAME << endl;
		return;
	}

	char type = 'I';
	rKey_t key = 0;
	rKey_t newKey = 0;
	double height, radius;

	/* Inserts */
	for (int i = 1; i < 100; i++){
		/* Write type of operation */
		type = 'I';
		file.write(&type, 1);

		/* Write key to insert */
		key = i;
		file.write((char*)&key, 8);

		/* Write height and radius */
		height = i;
		radius = i + 1;
		file.write((char*)&height, 8);
		file.write((char*)&radius, 8);

		/* Fill rest with zeros */
		newKey = 0;
		file.write((char*)&newKey, 8);
	}

	/* Print */
	type = 'P';
	key = 0;
	file.write(&type, 1);
	file.write((char*)&key, 8);
	file.write((char*)&key, 8);
	file.write((char*)&key, 8);
	file.write((char*)&key, 8);

	/* Updates */
	for (int i = 99; i > 0; i--){
		/* Write type of operation */
		type = 'U';
		file.write(&type, 1);

		/* Write key to update */
		key = 100 - i;
		file.write((char*)&key, 8);

		/* Write height and radius - 0 = dont change*/
		height = 0;
		radius = 0;
		file.write((char*)&height, 8);
		file.write((char*)&radius, 8);

		/* Set new key */
		newKey = i;
		file.write((char*)&newKey, 8);
	}

	/* Print */
	type = 'P';
	key = 0;
	file.write(&type, 1);
	file.write((char*)&key, 8);
	file.write((char*)&key, 8);
	file.write((char*)&key, 8);
	file.write((char*)&key, 8);

	file.close();

}

void BTree::findNeighbourNodes(position_t nodePos,
		position_t& left,
		position_t& right) {

	/* Get parent */
	TreeNode* node = manager.getNode(nodePos);
	if (node->m_parentPosition == ERROR){
		left = ERROR;
		right = ERROR;
		return;
	}

	TreeNode* parent = manager.getNode(node->m_parentPosition);

	/* Find neighbours  */
	auto it = parent->m_nodePointers.begin();
	left = ERROR;
	right = ERROR;

	int i = 0;
	for (auto ptr : parent->m_nodePointers){

		if (ptr == nodePos){

			/* Save leftmost neighbour position */
			if (i){
				advance(it, i-1);
				if ((*it) != nodePos)
					left = (*it);
			}

			/* Save rightmost neighbour position */
			advance(it, 2);

			if (it != parent->m_nodePointers.end())
				right = (*it);

			break;
		}

		i++;
	}
	return;
}
