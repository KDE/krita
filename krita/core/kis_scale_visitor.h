/*
 *  copyright (c) 2004, 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#ifndef KIS_SCALE_VISITOR_H_
#define KIS_SCALE_VISITOR_H_

#include "kis_types.h"
#include "kis_progress_subject.h"

class KisPaintDevice;
class KisProgressDisplayInterface;

enum enumFilterType {
	BOX_FILTER,
	TRIANGLE_FILTER,
	BELL_FILTER,
	B_SPLINE_FILTER,
	FILTER,
	LANCZOS3_FILTER,
	MITCHELL_FILTER
};

class KisScaleFilterStrategy {
	public:
		KisScaleFilterStrategy() {}
		virtual ~KisScaleFilterStrategy() {}

		virtual double valueAt(double t) const = 0;
                double support() { return supportVal;};
	protected:
		double supportVal;
};

class KisSimpleScaleFilterStrategy : public KisScaleFilterStrategy {
	public:
		KisSimpleScaleFilterStrategy() {supportVal = 1.0;}
                virtual ~KisSimpleScaleFilterStrategy() {}

		virtual double valueAt(double t) const;
};

class KisBoxScaleFilterStrategy : public KisScaleFilterStrategy {
	public:
		KisBoxScaleFilterStrategy() {supportVal = 0.5;}
                virtual ~KisBoxScaleFilterStrategy() {}

		virtual double valueAt(double t) const;
};

class KisTriangleScaleFilterStrategy : public KisScaleFilterStrategy {
	public:
		KisTriangleScaleFilterStrategy() {supportVal = 1.0;}
                virtual ~KisTriangleScaleFilterStrategy() {}

		virtual double valueAt(double t) const;
};

class KisBellScaleFilterStrategy : public KisScaleFilterStrategy {
	public:
		KisBellScaleFilterStrategy() {supportVal = 1.5;}
                virtual ~KisBellScaleFilterStrategy() {}

		virtual double valueAt(double t) const;
};

class KisBSplineScaleFilterStrategy : public KisScaleFilterStrategy {
	public:
		KisBSplineScaleFilterStrategy() {supportVal = 2.0;}
                virtual ~KisBSplineScaleFilterStrategy() {}

		virtual double valueAt(double t) const;
};

class KisLanczos3ScaleFilterStrategy : public KisScaleFilterStrategy {
	public:
		KisLanczos3ScaleFilterStrategy() {supportVal = 3.0;}
                virtual ~KisLanczos3ScaleFilterStrategy() {}

		virtual double valueAt(double t) const;
        private:
                double sinc(double x) const; 
};

class KisMitchellScaleFilterStrategy : public KisScaleFilterStrategy {
	public:
		KisMitchellScaleFilterStrategy() {supportVal = 2.0;}
                virtual ~KisMitchellScaleFilterStrategy() {}

		virtual double valueAt(double t) const;
};

class KisScaleVisitor : public KisProgressSubject {
        typedef KisProgressSubject super;  
        
        /* Structs for the image rescaling routine */
	class Contrib {
                public:	
                        Q_INT32 m_pixel;
                        double m_weight;
	};
 
	class ContribList {
	       public:
                        Q_INT32  n;  //number of contributors
                        Contrib *p; //pointer to list of contributions
	};

public:
        KisScaleVisitor();
        ~KisScaleVisitor();
        void visitKisPaintDevice(KisPaintDevice* dev);
        void scale(double sx, double sy, KisProgressDisplayInterface *m_progress, enumFilterType filterType = MITCHELL_FILTER);
private:
        KisPaintDevice* m_dev;
        
        
        /**
	 * calc_x_contrib()
	 *       
	 * Calculates the filter weights for a single target column.
	 * contribX->p must be freed afterwards.
	 *
	 * Returns -1 if error, 0 otherwise.
	 */
                
        int calcContrib(ContribList *contribX, double cale, double fwidth, int srcwidth, KisScaleFilterStrategy *filterStrategy, Q_INT32 i);

        ContribList * contrib;  //array of contribution lists

	// Implement KisProgressSubject
	bool m_cancelRequested;
        virtual void cancel() { m_cancelRequested = true; }

};

inline KisScaleVisitor::KisScaleVisitor()
{
}

inline KisScaleVisitor::~KisScaleVisitor()
{
}

inline void KisScaleVisitor::visitKisPaintDevice(KisPaintDevice* dev)
{
        m_dev=dev;
}
#endif // KIS_SCALE_VISITOR_H_
