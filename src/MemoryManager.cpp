/*
 *	MemoryManager.cpp
 *
 *  Created on: Nov 28, 2017
 *  Author:		Tomasz Rybicki
 */

#include "MemoryManager.h"

/*
 * Nodes are saved as pages. every node is identified with its position in file
 *
 * many Records are stored in one page
 * page consits of (Header 1B - full or not)(Record (key, height, radius))(Record...
 * the basic version assumes appening records to file one after another
 * the page to write is a member, when its full next page is created
 */

MemoryManager::MemoryManager(int degree)
	: m_nodesFile("Nodes.bin")
	, m_recordsFile("Records.bin")
	, m_degree(degree)
	, m_allocatedNodes()

{
	/* 2*d records */
	m_nodeEntrySize = (2 * m_degree) * sizeof(treeRecord);

	/* Each node can have 2*d + 1 node pointers */
	m_nodeEntrySize += ((2 * m_degree) + 1) * sizeof(position_t);

	/* Each node entry has a header consisting of:
	 *	- Parent node position
	 */
	m_nodeEntrySize += sizeof(position_t);

	/* Create files */
	ofstream file(m_recordsFile, ios::out|ios::binary|ios::app);

	if (!file.good()){
		cerr << "File cannot be created: " << m_recordsFile << endl;
		return;
	}

	file.close();

	ofstream file2(m_nodesFile, ios::out|ios::binary|ios::app);

	if (!file2.good()){
		cerr << "File cannot be created: " << m_nodesFile << endl;
		return;
	}

	file2.close();
}

MemoryManager::~MemoryManager() {
	for (auto it = m_allocatedRecords.begin(); it != m_allocatedRecords.end();){
		deallocateBlock(it->first);
		it++;
	}
}

TreeNode* MemoryManager::getNode(position_t nodePosition) {
	/* Check if is already allocated */
	map<position_t, TreeNode*>::iterator it;
	it = m_allocatedNodes.find(nodePosition);

	if(it != m_allocatedNodes.end()){
		return it->second;
	}

	ifstream file(m_nodesFile, ios::in|ios::binary);
	treeRecord tr;

	/* File cannot be opened */
	if (!file.good()){
		cerr << "File cannot be opened: " << m_nodesFile << endl;
		return 0;
	}

	/* Find nodes position in file */
	file.seekg(nodePosition);

	/* Allocate new node */
	TreeNode* node = new TreeNode;
	node->m_position = nodePosition;

	/* Read data */
	char* bytesStart = new char[m_nodeEntrySize];
	char* bytes = bytesStart;
	file.read(bytes, m_nodeEntrySize);

	node->m_parentPosition = *((position_t*)bytes);
	bytes += sizeof(position_t);

	for (int i = 0; i < (2 * m_degree); i++){
		tr.key = *((rKey_t*)bytes);
		bytes += sizeof(rKey_t);
		tr.position = *((position_t*)bytes);
		bytes += sizeof(position_t);

		/* Dont create empty records */
		if(tr.key == 0){
			break;
		}

		node->m_treeRecords.push_back(tr);
	}

	/* Move to node pointers part */
	bytes = bytesStart + sizeof(position_t) + ((2 * m_degree) * (sizeof(position_t) + sizeof (rKey_t)));
	position_t pos;

	for (int i = 0; i < (2 * m_degree); i++){
		pos = *((position_t*)bytes);
		bytes += sizeof(position_t);

		if(tr.key == 0 || tr.position == 0){
			break;
		}

		node->m_nodePointers.push_back(pos);
	}

	delete[] bytesStart;
	file.close();

	/* Update allocation map */
	m_allocatedNodes[node->m_position] = node;

	return node;
}

void MemoryManager::deallocateNode(TreeNode* node) {
	ofstream file(m_nodesFile, ios::out|ios::binary);

	/* File cannot be opened */
	if (!file.good()){
		cerr << "File cannot be opened: " << m_nodesFile << endl;
		return;
	}

	/* Find nodes position in file */
	file.seekp(node->getPosition());

	/* Write node to file */
	char* bytesStart = new char[m_nodeEntrySize]();
	char* bytes = bytesStart;

	position_t parent = node->getParentPosition();
	*((position_t*)bytes) = parent;
	bytes += sizeof(position_t);

	for (auto it : node->getTreeRecords()){
		*((rKey_t*)bytes) = it.key;
		bytes += sizeof(rKey_t);

		*((position_t*)bytes) = it.position;
		bytes += sizeof(position_t);
	}

	/* Leave zeros if node is not full - move to node pointers part */
	bytes = bytesStart + sizeof(position_t) + ((2 * m_degree) * (sizeof(position_t) + sizeof (rKey_t)));

	for (auto it : node->getPointers()){
			*((position_t*)bytes) = it;
			bytes += sizeof(position_t);
	}

	file.write(bytesStart, m_nodeEntrySize);

	file.close();
	delete[] bytesStart;

	/* Update allocation map */
	map<position_t, TreeNode*>::iterator it;
	it = m_allocatedNodes.find(node->m_position);
	if(it != m_allocatedNodes.end()){
		m_allocatedNodes.erase(it);
	}

	/* Free node */
	delete node;
}

TreeNode* MemoryManager::newNode() {
	ofstream file(m_nodesFile, ios::out|ios::app|ios::binary);

	/* File cannot be opened */
	if (!file.good()){
		cerr << "File cannot be opened: " << m_nodesFile << endl;
		return 0;
	}

	/* Save current position as node identificator */
	position_t pos = file.tellp();

	/* Zero out the new node in file */
	char* zero = new char[m_nodeEntrySize]();

	file.write(zero, sizeof(m_nodeEntrySize));

	file.close();
	delete[] zero;

	return getNode(pos);
}

void MemoryManager::updateNode(TreeNode* node) {
	ofstream file(m_nodesFile, ios::out|ios::binary);

	/* File cannot be opened */
	if (!file.good()){
		cerr << "File cannot be opened: " << m_nodesFile << endl;
		return;
	}

	/* Find nodes position in file */
	file.seekp(node->getPosition());

	/* Write node to file */
	char* bytesStart = new char[m_nodeEntrySize]();
	char* bytes = bytesStart;

	position_t parent = node->getParentPosition();
	*((position_t*)bytes) = parent;
	bytes += sizeof(position_t);

	for (auto it : node->getTreeRecords()){
		*((rKey_t*)bytes) = it.key;
		bytes += sizeof(rKey_t);

		*((position_t*)bytes) = it.position;
		bytes += sizeof(position_t);
	}

	/* Leave zeros if node is not full - move to node pointers part */
	bytes = bytesStart + sizeof(position_t) + ((2 * m_degree) * (sizeof(position_t) + sizeof (rKey_t)));

	for (auto it : node->getPointers()){
			*((position_t*)bytes) = it;
			bytes += sizeof(position_t);
	}

	file.write(bytesStart, m_nodeEntrySize);

	file.close();
	delete[] bytesStart;
}


Record* MemoryManager::newRecord() {
	/* Allocate record */
	Record* record = new Record(0,0,0);

	/* Get free page into memory */
	getBlock(m_freeBlock);

	/* Add record to page */
	m_allocatedRecords[m_freeBlock].push_back(record);

	/* If we have filled the page - update free page pointer */
	if(m_allocatedRecords[m_freeBlock].size() == RECORDS_PER_PAGE){
		m_freeBlock += RECORD_PAGE_SIZE;
	}

	return record;
}

Record* MemoryManager::getRecord(position_t position, rKey_t key){
	/* Get page with record */
	getBlock(position);

	/* Look for record with given ID */
	for (auto record : m_allocatedRecords[position]){
		if (record->m_id == key){
			return record;
		}
	}

	return NULL;
}

void MemoryManager::syncRecords() {
	ofstream file(m_recordsFile, ios::out|ios::binary);

	/* File cannot be opened */
	if (!file.good()){
		cerr << "File cannot be opened: " << m_recordsFile << endl;
		return;
	}

	/* Buffer for page */
	char* bytesStart = new char[RECORD_PAGE_SIZE]();
	char* bytes = bytesStart;

	/* Update each page */
	for (auto page : m_allocatedRecords){
		bytes = bytesStart;

		/* Set header if page is full */
		if (page.second.size() >= RECORDS_PER_PAGE){
			*((char*)bytes) = 1U;
		}
		bytes += sizeof(char);

		for (auto record : page.second){
			*((rKey_t*)bytes) = record->m_id;
			bytes += sizeof(rKey_t);
			*((double*)bytes) = record->m_height;
			bytes += sizeof(double);
			*((double*)bytes) = record->m_radius;
			bytes += sizeof(double);
		}

		/* Write page */
		file.seekp(page.first);

		file.write(bytesStart, RECORD_PAGE_SIZE);
	}

	file.close();
	delete[] bytesStart;
}

void MemoryManager::getBlock(position_t position)	{
		/* Check if block is already in memory */
		if (m_allocatedRecords.find(position) != m_allocatedRecords.end()){
			return;
		};

		ifstream file(m_recordsFile, ios::in|ios::binary);

		/* File cannot be opened */
		if (!file.good()){
			cerr << "File cannot be opened: " << m_nodesFile << endl;
			return;
		}

		/* Load the block from records file */
		char* bytesStart = new char[RECORD_PAGE_SIZE]();
		char* bytes = bytesStart;

		file.seekg(position);

		/* Read data */
		file.read(bytesStart, RECORD_PAGE_SIZE);

		/* Ommit header - true/false value if page is not full */
		bytes += sizeof(char);

		/* Find records */
		Record r(0,0,0);
		Record* rPtr;
		for (int i = 0; i < RECORDS_PER_PAGE; i++){
			r.m_id = *((rKey_t*)bytes);
			bytes += sizeof(rKey_t);
			r.m_height = *((double*)bytes);
			bytes += sizeof(double);
			r.m_radius = *((double*)bytes);
			bytes += sizeof(double);

			/* Found record */
			if (r.m_id != 0){
				rPtr = new Record(0,0,0);
				rPtr->m_id = r.m_id;
				rPtr->m_height = r.m_height;
				rPtr->m_radius = r.m_radius;

				m_allocatedRecords[position].push_back(rPtr);
			}
		}

		delete[] bytesStart;
		file.close();
}

void MemoryManager::deallocateBlock(position_t position) {
	ofstream file(m_recordsFile, ios::out|ios::binary);

	/* File cannot be opened */
	if (!file.good()){
		cerr << "File cannot be opened: " << m_recordsFile << endl;
		return;
	}

	/* Buffer for page */
	char* bytesStart = new char[RECORD_PAGE_SIZE]();
	char* bytes = bytesStart;

	/* Update each page */
	bytes = bytesStart;

	/* Set header if page is full */
	if (m_allocatedRecords[position].size() >= RECORDS_PER_PAGE){
		*((char*)bytes) = 1U;
	}
	bytes += sizeof(char);

	for (auto record : m_allocatedRecords[position]){
		*((rKey_t*)bytes) = record->m_id;
		bytes += sizeof(rKey_t);
		*((double*)bytes) = record->m_height;
		bytes += sizeof(double);
		*((double*)bytes) = record->m_radius;
		bytes += sizeof(double);
	}

	/* Write page */
	file.seekp(position);
	file.write(bytesStart, RECORD_PAGE_SIZE);

	/* Free memory */
	for (auto record : m_allocatedRecords[position]){
		delete record;
	}
	map<position_t, list<Record*>>::iterator it = m_allocatedRecords.find(position);
    m_allocatedRecords.erase(it);
	delete[] bytesStart;

	file.close();
}

void MemoryManager::printRecords() {
	cout << "Pages in memory: " << m_allocatedRecords.size() << endl << endl;

	for (auto page : m_allocatedRecords){
		cout << "Page position: " << page.first << " Records:" << endl;

		for (auto record : page.second){
			cout << " Record( key:" << record->m_id;
			cout << " height:"<<record->m_height;
			cout << " radius:"<<record->m_radius;
			cout << " )" << endl;
		}
		cout << endl;
	}
}
