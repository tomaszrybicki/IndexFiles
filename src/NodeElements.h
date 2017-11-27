/*
 * NodeElements.h
 *
 *  Created on: Nov 26, 2017
 *  Author: Tomasz Rybicki
 */

#include "Defines.h"
#include <vector>

using namespace std;

class TreeNode;

/* Node element is either a pointer to other node
 * or a record's key-location pair
 *
 * It's fields are in an array because it allows
 * to optimize memory usage while still using one class
 * (for use in STL list)  */

class NodeElement{
public:

	/* Constructor for pointer to node */
	NodeElement(unsigned long long nodePosition);

	/* Constructor for record's index */
	NodeElement(unsigned long long recordPosition, unsigned long long key);

	~NodeElement();

	/* Returns record's key value
	 * or WRONG_ELEMENT if used on pointer to node */
	unsigned long long getKey();

	/* Returns pointer to a given node,
	 *  which is loaded to memory if necessary */
	TreeNode* getNode();

private:
	vector<unsigned long long> m_fields;
};
