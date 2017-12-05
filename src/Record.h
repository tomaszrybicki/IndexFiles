/*
 *	Record.h
 *
 *  Created on: Oct 30, 2017
 *  Author:		Tomasz Rybicki
 */

#ifndef RECORD_H_
#define RECORD_H_

#include <math.h>
#include "Defines.h"

/*
 * Class describing a record in database file
 * which represents a cone
 */
class Record {
public:
	friend class MemoryManager;
	Record(double h, double r, unsigned long long id);
	virtual ~Record();
	double getVolume();

	double getHeight() const {
		return m_height;
	}

	/*
	 * This function returns the volume of the cone
	 * calculated using V = 1/3 * r^2 * h
	 */
	double getRadius() const {
		return m_radius;
	}

	rKey_t getID(){
		return m_id;
	}

	void setId(rKey_t id) {
		m_id = id;
	}

	void setHeight(double height) {
		m_height = height;
	}

	void setRadius(double radius) {
		m_radius = radius;
	}

private:
	double m_height;
	double m_radius;
	rKey_t m_id;
};

#endif /* RECORD_H_ */
