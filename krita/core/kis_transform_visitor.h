/*
 *  copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */

#ifndef KIS_TRANSFORM_VISITOR_H_
#define KIS_TRANSFORM_VISITOR_H_

#include "kis_types.h"
#include "kis_progress_subject.h"

class KisPaintDevice;
class KisProgressDisplayInterface;

/*
enum enumFilterType {
	BOX_FILTER,
	TRIANGLE_FILTER,
	BELL_FILTER,
	B_SPLINE_FILTER,
	FILTER,
	LANCZOS3_FILTER,
	MITCHELL_FILTER
};
*/
class KisFilterStrategy {
	public:
		KisFilterStrategy() {}
		virtual ~KisFilterStrategy() {}

		virtual double valueAt(double t) const = 0;
		double support() { return supportVal;};
	protected:
		double supportVal;
};

class KisSimpleFilterStrategy : public KisFilterStrategy {
	public:
		KisSimpleFilterStrategy() {supportVal = 1.0;}
		virtual ~KisSimpleFilterStrategy() {}

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

class KisTransformVisitor : public KisProgressSubject {
	typedef KisProgressSubject super;

public:
	KisTransformVisitor();
	~KisTransformVisitor();
	void visitKisPaintDevice(KisPaintDevice* dev);
	void transformx(Q_INT32 scale, Q_INT32 scaleDenom, Q_INT32  shear, Q_INT32 dx,   KisProgressDisplayInterface *m_progress, KisFilterStrategy *filterStrategy);

	void transform(Q_INT32  xscale, Q_INT32  yscale, 
			Q_INT32  xshear, Q_INT32  yshear, Q_INT32  denominator,
			Q_INT32  xtranslate, Q_INT32  ytranslate,
 			KisProgressDisplayInterface *m_progress, enumFilterType filterType = MITCHELL_FILTER);
private:
	KisPaintDevice* m_dev;
	
	// Implement KisProgressSubject
	bool m_cancelRequested;
	virtual void cancel() { m_cancelRequested = true; }
};

inline KisTransformVisitor::KisTransformVisitor()
{
}

inline KisTransformVisitor::~KisTransformVisitor()
{
}

inline void KisTransformVisitor::visitKisPaintDevice(KisPaintDevice* dev)
{
	m_dev=dev;
}
#endif // KIS_TRANSFORM_VISITOR_H_
