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
}

MemoryManager::~MemoryManager() {

}

TreeNode* MemoryManager::getNode(position_t nodePosition) {
	/* Check if is already allocated */
	map<position_t, TreeNode*>::iterator it;
	it = m_allocatedNodes.find(nodePosition);

	if(it != m_allocatedNodes.end()){
		return it->second;
	}

	Bar b3;
	if(it != m.end())
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

void MemoryManager::syncNodes() {
}

/* Caller has to set id for record before its deallocated */
Record* MemoryManager::newRecord() {
	/* Allocate record */
	Record* record = new Record(0,0,0);

	/* If the free page is in memory */
	if (m_allocatedRecords.find(m_freeBlock) != m_allocatedRecords.end()){
		m_allocatedRecords[m_freeBlock].push_back(record);

		/* If we have filled the page */
		if(m_allocatedRecords[m_freeBlock].size() == RECORDS_PER_PAGE){
			m_freeBlock += RECORD_PAGE_SIZE;
		}

		return record;
	};

	getBlock(); and do the part above
//	ofstream file(m_recordsFile, ios::out|ios::app|ios::binary);
//
//	/* File cannot be opened */
//	if (!file.good()){
//		cerr << "File cannot be opened: " << m_nodesFile << endl;
//		return 0;
//	}
//
//	/* Load the free block from records file */
//	char* bytesStart = new char[RECORD_PAGE_SIZE]();
//	char* bytes = bytesStart;
//
//	/* Find nodes position in file */
//	file.seekp(m_freeBlock);
//
//	/* Read data */
//	file.read(bytesStart, RECORD_PAGE_SIZE);
//
//	/* Ommit header - true/false value if page is not full */
//	bytes += sizeof(char);
//
//	/* Find place for record */
//	Record r;
//	for (int i = 0; i < RECORDS_PER_PAGE; i++){
//		r.m_id = *((rKey_t*)bytes);
//		bytes += sizeof(rKey_t);
//		r.m_height = *((double*)bytes);
//		bytes += sizeof(double);
//		r.m_radius = *((double*)bytes);
//		bytes += sizeof(double);
//
//		/* Found empty place */
//		if (r.m_id == 0){
//
//		}else{
//
//			m_allocatedRecords[m_freeBlock].push_back(r)
//		}
//
//
//		//check if full
//	}
//
//	//delete

}

Record* MemoryManager::getRecord(position_t position, rKey_t key){
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

		file.seekg(m_freeBlock);

		/* Read data */
		file.read(bytesStart, RECORD_PAGE_SIZE);

		/* Ommit header - true/false value if page is not full */
		bytes += sizeof(char);

		/* Find records */
		Record r;
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
				rPtr = new Record;
				rPtr->m_id = r.m_id;
				rPtr->m_height = r.m_height;
				rPtr->m_radius = r.m_radius;

				m_allocatedRecords[position].push_back(rPtr);
			}
		}

		delete[] bytesStart;
		file.close();
}
