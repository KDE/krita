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
#ifndef KIS_RAINDROPS_FILTER_VISITOR_H_
#define KIS_RAINDROPS_FILTER_VISITOR_H_

#include "kis_types.h"
#include "kis_progress_subject.h"

class KisPaintDevice;
class KisProgressDisplayInterface;

class KisRainDropsFilterVisitor : public KisProgressSubject {
        typedef KisProgressSubject super;  
        
        /* Structs for the image rescaling routine */
	
public:
        KisRainDropsFilterVisitor();
        ~KisRainDropsFilterVisitor();
        void visitKisPaintDevice(KisPaintDevice* dev);
        void rainDropsFilter(Q_UINT32 dropSize, Q_UINT32 number, Q_UINT32 fishEyes, KisProgressDisplayInterface *m_progress);
private:
        KisPaintDevice* m_dev;
        
	// Implement KisProgressSubject
	bool m_cancelRequested;
        virtual void cancel() { m_cancelRequested = true; }
        
        //Raindrops algorithm
        void   rainDrops(QUANTUM *data, int Width, int Height, int DropSize, int Amount, int Coeff);
        bool** CreateBoolArray (uint Columns, uint Rows);
        void   FreeBoolArray (bool** lpbArray, uint Columns);
        uchar  LimitValues (int ColorValue);
};

inline KisRainDropsFilterVisitor::KisRainDropsFilterVisitor()
{
}

inline KisRainDropsFilterVisitor::~KisRainDropsFilterVisitor()
{
}

inline void KisRainDropsFilterVisitor::visitKisPaintDevice(KisPaintDevice* dev)
{
        m_dev=dev;
}
#endif // KIS_RAINDROPS_FILTER_VISITOR_H_
