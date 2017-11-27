/*
 *	Record.cpp
 *
 *  Created on: Oct 30, 2017
 *  Author:		Tomasz Rybicki
 */

#include "Record.h"
#include <iostream>

Record::Record(double h, double r, unsigned long long id)
	: m_height(h)
	, m_radius(r)
	, m_id(id)
{
}

Record::~Record() {}

double Record::getVolume(){
	double volume = 0;
	volume = ( (double)1/(double)3 ) * M_PI * m_radius * m_radius * m_height;

	return volume;
}
