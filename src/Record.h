/*
 *	Record.h
 *
 *  Created on: Oct 30, 2017
 *  Author:		Tomasz Rybicki
 */

#ifndef RECORD_H_
#define RECORD_H_

#include <math.h>

/*
 * Class describing a record in database file
 * which represents a cone
 */
class Record {
public:
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

	unsigned long long getID(){
		return m_id;
	}

private:
	double m_height;
	double m_radius;
	unsigned long long m_id;
};

#endif /* RECORD_H_ */
