/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef KIS_FILTER_STRATEGY_H_
#define KIS_FILTER_STRATEGY_H_

#include "kis_types.h"


enum enumFilterType {
	BOX_FILTER,
	TRIANGLE_FILTER,
	BELL_FILTER,
	B_SPLINE_FILTER,
	HERMITE_FILTER,
	LANCZOS3_FILTER,
	MITCHELL_FILTER
};

class KisFilterStrategy {
	public:
		KisFilterStrategy() {}
		virtual ~KisFilterStrategy() {}

		virtual double valueAt(double t) const = 0;
		virtual Q_UINT32 intValueAt(Q_INT32 ) const {return 0;};
		double support() { return supportVal;};
	protected:
		double supportVal;
};

class KisHermiteFilterStrategy : public KisFilterStrategy {
	public:
		KisHermiteFilterStrategy() {supportVal = 1.0;}
		virtual ~KisHermiteFilterStrategy() {}
		virtual Q_UINT32 intValueAt(Q_INT32 t) const;

		virtual double valueAt(double t) const;
};

class KisBoxFilterStrategy : public KisFilterStrategy {
	public:
		KisBoxFilterStrategy() {supportVal = 0.5;}
		virtual ~KisBoxFilterStrategy() {}

		virtual double valueAt(double t) const;
};

class KisTriangleFilterStrategy : public KisFilterStrategy {
	public:
		KisTriangleFilterStrategy() {supportVal = 1.0;}
		virtual ~KisTriangleFilterStrategy() {}

		virtual double valueAt(double t) const;
};

class KisBellFilterStrategy : public KisFilterStrategy {
	public:
		KisBellFilterStrategy() {supportVal = 1.5;}
		virtual ~KisBellFilterStrategy() {}

		virtual double valueAt(double t) const;
};

class KisBSplineFilterStrategy : public KisFilterStrategy {
	public:
		KisBSplineFilterStrategy() {supportVal = 2.0;}
		virtual ~KisBSplineFilterStrategy() {}

		virtual double valueAt(double t) const;
};

class KisLanczos3FilterStrategy : public KisFilterStrategy {
	public:
		KisLanczos3FilterStrategy() {supportVal = 3.0;}
		virtual ~KisLanczos3FilterStrategy() {}

		virtual double valueAt(double t) const;
	private:
		double sinc(double x) const; 
};

class KisMitchellFilterStrategy : public KisFilterStrategy {
	public:
		KisMitchellFilterStrategy() {supportVal = 2.0;}
		virtual ~KisMitchellFilterStrategy() {}

		virtual double valueAt(double t) const;
};

#endif // KIS_FILTER_STRATEGY_H_
