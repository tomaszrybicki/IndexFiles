/*
 *	MemoryManager.cpp
 *
 *  Created on: Nov 28, 2017
 *  Author:		Tomasz Rybicki
 */

#include "MemoryManager.h"

uint64_t recordReads;
uint64_t recordWrites;
uint64_t indexReads;
uint64_t indexWrites;

/*
 * Nodes are saved as pages. every node is identified with its position in file
 *
 * Many records are stored in one page
 * page consists of (Header 1B - full or not)(Record (key, height, radius))(Record...
 * the basic version assumes appening records to file one after another
 * the position of page to write is a member, when its full next page is created
 */

MemoryManager::MemoryManager(int degree, int limit)
	: m_nodesFile("Nodes.bin")
	, m_recordsFile("Records.bin")
	, m_degree(degree)
	, m_allocatedNodes()
	, m_allocatedNodesCopy()
	, m_allocatedRecords()
	, m_allocatedRecordsCopy()
	, m_memoryLimit(limit)
	, m_pagesInMemory(0)
	, m_nodesInMemory(0)
	, m_globalSid(0)
	, m_rootNodePosition(ERROR)
{
	/* 2 * degree records */
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

	loadState();
}

void MemoryManager::updatePathToRoot(position_t nodePos) {
	if (nodePos == ERROR){
		return;
	}

	TreeNode* node = getNode(nodePos);
	position_t parent = node->m_parentPosition;

	while (parent != ERROR){
		node = getNode(parent);
		parent = node->m_parentPosition;
	}

	return;
}

MemoryManager::~MemoryManager() {
	for (auto it = m_allocatedRecords.begin(); it != m_allocatedRecords.end();){
		deallocateBlock(it->first);
		it++;
	}

	m_allocatedRecords.clear();
	m_allocatedRecordsCopy.clear();


	for (auto it = m_allocatedNodes.begin(); it != m_allocatedNodes.end();){
		if (it->second)
			deallocateNode(it->second);
		it++;
	}

	m_allocatedNodes.clear();
	m_allocatedNodesCopy.clear();

	cout << endl << "End statistics:" << endl;
	cout << "[Reads] record: " << recordReads << " Index reads: "<< indexReads << " total: " << recordReads + indexReads << endl;
	cout << "[Writes] record:" << recordWrites << " index: "<< indexWrites << " total: " << recordWrites + indexWrites << endl;

	saveState();
}

TreeNode* MemoryManager::getNode(position_t nodePosition) {
	updateNodeStats(nodePosition);

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

	m_nodesInMemory++;
	indexReads++;

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

		/* Don't create empty records */
		if(tr.key == 0){
			break;
		}

		node->m_treeRecords.push_back(tr);
	}

	/* Move to node pointers part */
	bytes = bytesStart + sizeof(position_t) + ((2 * m_degree) * (sizeof(position_t) + sizeof (rKey_t)));
	position_t pos;

	for (int i = 0; i < (2 * m_degree + 1); i++){
		pos = *((position_t*)bytes);
		bytes += sizeof(position_t);

		if(pos)
			node->m_nodePointers.push_back(pos);
	}

	delete[] bytesStart;
	file.close();

	/* Update allocation map */
	m_allocatedNodes[node->m_position] = node;
	m_allocatedNodesCopy[node->m_position] = *node;

	maintance();

	return node;
}

void MemoryManager::deallocateNode(TreeNode* node) {
	/* If node was changed */
	if (!(m_allocatedNodesCopy[node->m_position] == (*node))){

		ofstream file(m_nodesFile, ios::out|ios::binary|ios::in);

		/* File cannot be opened */
		if (!file.good()){
			cerr << "File cannot be opened: " << m_nodesFile << endl;
			return;
		}

		indexWrites++;

		/* Find nodes position in file */
		file.seekp(node->getPosition());

		/* Write node to file */
		char* bytesStart = new char[m_nodeEntrySize]();
		char* bytes = bytesStart;

		/* Write parent position */
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

	m_nodesInMemory--;


	/* Update allocation mapS */

	m_allocatedNodes.erase(node->m_position);

	m_allocatedNodesCopy.erase(node->m_position);

	/* Update SIDs map */
	m_allocatedNodesSid.erase(node->m_position);

	/* Free node */
	if(node){
		node = 0;
	}
}

TreeNode* MemoryManager::newNode() {
	ofstream file(m_nodesFile, ios::out|ios::app|ios::binary);

	/* File cannot be opened */
	if (!file.good()){
		cerr << "File cannot be opened: " << m_nodesFile << endl;
		return 0;
	}

	indexWrites++;

	/* Save current position as node identificator */
	position_t pos = file.tellp();

	/* Zero out the new node in file */
	char* zero = new char[m_nodeEntrySize]();

	file.write(zero, m_nodeEntrySize);

	file.close();
	delete[] zero;

	TreeNode* node = getNode(pos);
	node->m_parentPosition = ERROR;
	return node;
}

void MemoryManager::updateNode(TreeNode* node) {
	ofstream file(m_nodesFile, ios::out|ios::binary);

	/* File cannot be opened */
	if (!file.good()){
		cerr << "File cannot be opened: " << m_nodesFile << endl;
		return;
	}

	indexWrites++;

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


Record* MemoryManager::newRecord(position_t* position) {
	/* Allocate record */
	Record* record = new Record(0,0,0);

	/* Get free page into memory */
	getBlock(m_freeBlock);

	/* Add record to page */
	m_allocatedRecords[m_freeBlock].push_back(record);
	m_allocatedRecordsCopy[m_freeBlock].push_back(*record);

	if(m_allocatedRecords[m_freeBlock].size()>2){
		cout << "Overfilled page!" << endl;
	}

	(*position) = m_freeBlock;

	updatePageStats(m_freeBlock);

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
			updatePageStats(m_freeBlock);
			return record;
		}
	}

	cerr << "Couldn't find record with id: " << key;
	cerr << " on page: " << position << endl;
	return NULL;
}

void MemoryManager::syncRecords() {
	ofstream file(m_recordsFile, ios::out|ios::binary);

	/* File cannot be opened */
	if (!file.good()){
		cerr << "File cannot be opened: " << m_recordsFile << endl;
		return;
	}

	recordWrites++;

	/* Buffer for page */
	char bytesStart[RECORD_PAGE_SIZE] = {};
	char* bytes = bytesStart;

	/* Update each page */
	for (auto const &page : m_allocatedRecords){
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
}

void MemoryManager::getBlock(position_t position){
	updatePageStats(position);

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

	m_pagesInMemory++;
	recordReads++;

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
			if(m_allocatedRecords[position].size()>2){
				cout << "Overfilled page!" << endl;
			}
			m_allocatedRecordsCopy[position].push_back(*rPtr);

		}
	}

	delete[] bytesStart;
	file.close();

	maintance();
}

void MemoryManager::deallocateBlock(position_t position) {
	bool changed = false;

	auto pageCopy = m_allocatedRecordsCopy[position];
	auto page = m_allocatedRecords[position];
	auto it1 = pageCopy.begin();
	auto it2 = page.begin();
	Record r1, r2;

	for(; it1 != pageCopy.end() && it2 != page.end(); ++it1, ++it2){
		r1 = (*it1);
		r2 = *(*it2);

		if (!(r1 == r2)){
			changed = true;
			break;
		}
	}


	if (changed){
		fstream file(m_recordsFile, ios::out|ios::binary|ios::in);

		/* File cannot be opened */
		if (!file.good()){
			cerr << "File cannot be opened: " << m_recordsFile << endl;
			return;
		}

		recordWrites++;

		/* Buffer for page */
		char bytesStart[RECORD_PAGE_SIZE]={};
		char* bytes = bytesStart;

		/* Update each page */
		bytes = bytesStart;

		/* Set header if page is full */
		if (m_allocatedRecords[position].size() >= RECORDS_PER_PAGE){
			*((char*)bytes) = 1U;
		}
		bytes += sizeof(char);

		for (auto record : m_allocatedRecords[position]){
			/* Boundary only for safety and debugging help */
			if((bytes - bytesStart) >= (unsigned int)RECORD_PAGE_SIZE){
					cerr << "Page overfilled while deallocating!" << endl;
					break;
			}

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

		file.close();
	}

	/* Free memory */
	m_pagesInMemory--;

	for (auto record : m_allocatedRecords[position]){
		delete record;
	}


}

void MemoryManager::printRecords() {
	cout << "Pages in memory: " << m_pagesInMemory<< endl << endl;

	for (auto page : m_allocatedRecords){
		cout << "Page position: " << page.first << ", " << page.second.size() << " Records:" << endl;

		for (auto record : page.second){
			if (record == NULL){
				cout << " Record (NULL!!!)" << endl;
				continue;
			}
			cout << " Record( key:" << record->m_id;
			cout << " height:"<<record->m_height;
			cout << " radius:"<<record->m_radius;
			cout << " )" << endl;
		}
		cout << endl;
	}
}

void MemoryManager::saveState() {
	ofstream file(STATE_FILE_NAME, ios::out|ios::binary);

	/* File cannot be opened */
	if (!file.good()){
		cerr << "File cannot be opened: " << m_recordsFile << endl;
		return;
	}

	/* Write location of block with space for records */
	file.write((char*)&m_freeBlock, sizeof(m_freeBlock));

	/* Write location of root node block */
	file.write((char*)&m_rootNodePosition, sizeof(m_rootNodePosition));

	file.close();
}

void MemoryManager::loadState() {
	ifstream file(STATE_FILE_NAME, ios::in|ios::binary);

	/* No previous state */
	if (!file.good()){
		cerr << "No previous state file found" << endl;

		/* Create empty node to not use position 0 */
		newNode();

		return;
	}

	/* Read data */
	file.read((char*)&m_freeBlock, sizeof(m_freeBlock));

	file.read((char*)&m_rootNodePosition, sizeof(m_rootNodePosition));

	file.close();
}

void MemoryManager::deleteRecord(position_t position, rKey_t key) {
	/* Get block with record */
	getBlock(position);

	auto itCopy = m_allocatedRecordsCopy[position].begin();
	for (auto record = m_allocatedRecords[position].begin();
			record != m_allocatedRecords[position].end();
			record++){

		if ((*record)->m_id == key){
			delete (*record);
			record = m_allocatedRecords[position].erase(record);

			m_allocatedRecordsCopy[position].erase(itCopy);
			break;
		}

		itCopy++;
	}
}

void MemoryManager::updateNodeStats(position_t position) {
	if (m_globalSid + 1 < m_globalSid){
		cout << "Global SID overflow!" << endl;
	}

	m_globalSid++;

	m_allocatedNodesSid[position] = m_globalSid;
}

void MemoryManager::updatePageStats(position_t position) {
	m_globalSid++;
	m_allocatedRecordsSid[position] = m_globalSid;
}

void MemoryManager::maintance() {
	position_t toDeallocate = 0;
	sequenceID_t min = ~0;

	if (m_pagesInMemory > (m_memoryLimit)){

		/* Look for least recently used page */
		for (auto page : m_allocatedRecordsSid){

			/* Smallest sequence id = least recently used */
			if (page.second < min){
				min = page.second;
				toDeallocate = page.first;
			}
		}

		deallocateBlock(toDeallocate);
		if (DEBUG == 2){
			cout << "Maintance removed page: " << toDeallocate << endl;
			cout << "Pages in memory: " << m_pagesInMemory<< endl;
			cout << "Nodes in memory: " << m_nodesInMemory<< endl << endl;

		}

		/* Update maps */
		m_allocatedRecords.erase(toDeallocate);
		m_allocatedRecordsCopy.erase(toDeallocate);
		m_allocatedRecordsSid.erase(toDeallocate);
	}

	min = ~0;

	if (m_nodesInMemory > (m_memoryLimit)){

		/* Look for least recently used node */
		for (auto node : m_allocatedNodesSid){
			if (node.second < min){
				min = node.second;
				toDeallocate = node.first;
			}
		}

		deallocateNode(getNode(toDeallocate));
		if (DEBUG == 2){
			cout << "Maintance removed node: " << toDeallocate<<endl;
		}

		/* Update maps */
//		m_allocatedNodes.erase(toDeallocate);
//		m_allocatedNodesSid.erase(toDeallocate);
//		m_allocatedNodesCopy.erase(toDeallocate);
	}

}
