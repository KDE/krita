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
#ifndef KIS_ROTATE_VISITOR_H_
#define KIS_ROTATE_VISITOR_H_

#include "kis_types.h"
#include "kis_progress_subject.h"

class QRect;
class KisPaintDeviceImpl;
class KisProgressDisplayInterface;

class KisRotateVisitor : public KisProgressSubject {
        typedef KisProgressSubject super;  
        
        /* Structs for the image rescaling routine */
    
public:
        KisRotateVisitor();
        ~KisRotateVisitor();

        void visitKisPaintDeviceImpl(KisPaintDeviceImpl* dev);

    
        void rotate(double angle, bool rotateAboutImageCentre, KisProgressDisplayInterface *progress);
        void shear(double angleX, double angleY, KisProgressDisplayInterface *progress);

private:
        KisPaintDeviceImplSP m_dev;

    // Implement KisProgressSubject
    bool m_cancelRequested;
        virtual void cancel() { m_cancelRequested = true; }

    void initProgress(Q_INT32 totalSteps);
    void incrementProgress();
    void setProgressDone();

    KisProgressDisplayInterface *m_progress;
    Q_INT32 m_progressStep;
    Q_INT32 m_progressTotalSteps;
    Q_INT32 m_lastProgressPerCent;

    KisPaintDeviceImplSP rotateRight90(KisPaintDeviceImplSP src);
    KisPaintDeviceImplSP rotateLeft90(KisPaintDeviceImplSP src);
    KisPaintDeviceImplSP rotate180(KisPaintDeviceImplSP src);
    KisPaintDeviceImplSP rotate(KisPaintDeviceImplSP src, double angle, KisPoint centreOfRotation);

    KisPaintDeviceImplSP xShear(KisPaintDeviceImplSP src, double shearX);
    KisPaintDeviceImplSP yShear(KisPaintDeviceImplSP src, double shearY);

};

inline KisRotateVisitor::KisRotateVisitor()
{
}

inline KisRotateVisitor::~KisRotateVisitor()
{
}

inline void KisRotateVisitor::visitKisPaintDeviceImpl(KisPaintDeviceImpl* dev)
{
        m_dev = dev;
}
#endif // KIS_ROTATE_VISITOR_H_
