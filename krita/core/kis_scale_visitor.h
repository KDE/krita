/*
 *  copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

class KisScaleVisitor : public KisProgressSubject {
        typedef KisProgressSubject super;  
        
        /* Structs for the image rescaling routine */
	struct CONTRIB {
		Q_INT32 m_pixel;
		double m_weight;
	};
 
	struct CLIST {
		Q_INT32  n;  //number of contributors
		CONTRIB *p; //pointer to list of contributions
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
                
        int calc_x_contrib(CLIST *contribX, double xcale, double fwidth, int dstwidth, int srcwidth, double (KisScaleVisitor::*filterf)(double), Q_INT32 i);

	/* scaling filter function definitions */
	double filter(double t);
	double box_filter(double t);
	double triangle_filter(double t);
	double bell_filter(double t);
	double B_spline_filter(double t);
	double sinc(double x);
	double Lanczos3_filter(double t);
	double Mitchell_filter(double t);

        CLIST * contrib;  //array of contribution lists

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
