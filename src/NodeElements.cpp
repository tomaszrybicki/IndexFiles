/*
 * NodeElements.cpp
 *
 *  Created on: Nov 26, 2017
 *  Author: Tomasz Rybicki
 */

#include "NodeElements.h"

NodeElement::NodeElement(unsigned long long nodePosition) {
	/* Initialize pointed to node position */
	m_fields.push_back(nodePosition);
}

NodeElement::NodeElement( unsigned long long recordPosition, unsigned long long key) {
	/* Initialize pointed to record position */
	m_fields.push_back(recordPosition);

	/* Set key */
	m_fields.push_back(key);
}

NodeElement::~NodeElement() {
}

unsigned long long NodeElement::getKey() {
	if (m_fields.size() == 2){
		return m_fields[1];
	}else{
		return WRONG_ELEMENT;
	}
}

TreeNode* NodeElement::getNode() {
	return 0;
}
