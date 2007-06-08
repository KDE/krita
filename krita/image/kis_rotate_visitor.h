/*
 *  copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_ROTATE_VISITOR_H_
#define KIS_ROTATE_VISITOR_H_


#include "kis_types.h"
#include "kis_progress_subject.h"

class KisPaintDevice;
class KisProgressDisplayInterface;

class KRITAIMAGE_EXPORT KisRotateVisitor : public KisProgressSubject {
    typedef KisProgressSubject super;

    /* Structs for the image rescaling routine */

public:
    KisRotateVisitor();
    ~KisRotateVisitor();

    void visitKisPaintDevice(KisPaintDevice* dev);

    /**
     * Rotate the paint device with the specified angle around the
     * center of the paint device
     */
    void rotate(double angle, KisProgressDisplayInterface *progress);

    /**
     * rotate the paintdevice with the specified angle around the
     * middle of width and height.
     */
    void rotate(double angle, qint32 width, qint32 height, KisProgressDisplayInterface *progress);
    void shear(double angleX, double angleY, KisProgressDisplayInterface *progress);

private:

    void rotate( double angle, QPointF center, KisProgressDisplayInterface *progress);

    KisPaintDeviceSP m_dev;

    // Implement KisProgressSubject
    bool m_cancelRequested;
    virtual void cancel() { m_cancelRequested = true; }

    void initProgress(qint32 totalSteps);
    void incrementProgress();
    void setProgressDone();

    KisProgressDisplayInterface *m_progress;
    qint32 m_progressStep;
    qint32 m_progressTotalSteps;
    qint32 m_lastProgressPerCent;

    KisPaintDeviceSP rotateRight90(KisPaintDeviceSP src);
    KisPaintDeviceSP rotateLeft90(KisPaintDeviceSP src);
    KisPaintDeviceSP rotate180(KisPaintDeviceSP src);
    KisPaintDeviceSP rotate(KisPaintDeviceSP src, double angle, QPointF centerOfRotation);

    KisPaintDeviceSP xShear(KisPaintDeviceSP src, double shearX);
    KisPaintDeviceSP yShear(KisPaintDeviceSP src, double shearY);

};

inline KisRotateVisitor::KisRotateVisitor()
{
}

inline KisRotateVisitor::~KisRotateVisitor()
{
}

inline void KisRotateVisitor::visitKisPaintDevice(KisPaintDevice* dev)
{
    m_dev = dev;
}
#endif // KIS_ROTATE_VISITOR_H_
