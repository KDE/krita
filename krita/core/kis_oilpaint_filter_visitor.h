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
#ifndef KIS_OILPAINT_FILTER_VISITOR_H_
#define KIS_OILPAINT_FILTER_VISITOR_H_

#include "kis_types.h"
#include "kis_progress_subject.h"

class KisPaintDevice;
class KisProgressDisplayInterface;

class KisOilPaintFilterVisitor : public KisProgressSubject {
        typedef KisProgressSubject super;  
        
        /* Structs for the image rescaling routine */
	
public:
        KisOilPaintFilterVisitor();
        ~KisOilPaintFilterVisitor();
        void visitKisPaintDevice(KisPaintDevice* dev);
        void oilPaintFilter(Q_UINT32 brushSize, Q_UINT32 smooth, KisProgressDisplayInterface *m_progress);
private:
        KisPaintDevice* m_dev;
        
	// Implement KisProgressSubject
	bool m_cancelRequested;
        virtual void cancel() { m_cancelRequested = true; }

        // oilpaint algorithm
        void OilPaint(QUANTUM* data, int w, int h, int BrushSize, int Smoothness, KisProgressDisplayInterface *m_progress);
        inline uint MostFrequentColor(uchar* Bits, int Width, int Height, int X, 
                                  int Y, int Radius, int Intensity);                           
        // Function to calcule the color intensity and return the luminance (Y)
        // component of YIQ color model.
        inline uint GetIntensity(uint Red, uint Green, uint Blue)
           { return ((uint)(Red * 0.3 + Green * 0.59 + Blue * 0.11)); } 
};

inline KisOilPaintFilterVisitor::KisOilPaintFilterVisitor()
{
}

inline KisOilPaintFilterVisitor::~KisOilPaintFilterVisitor()
{
}

inline void KisOilPaintFilterVisitor::visitKisPaintDevice(KisPaintDevice* dev)
{
        m_dev=dev;
}
#endif // KIS_OILPAINT_FILTER_VISITOR_H_
