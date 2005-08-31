/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
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
#ifndef KIS_FILL_PAINTER_H_
#define KIS_FILL_PAINTER_H_

#include <qbrush.h>
#include <qfontinfo.h>
#include <qfontmetrics.h>
#include <qpen.h>
#include <qregion.h>
#include <qwmatrix.h>
#include <qimage.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qpointarray.h>
#include <qstring.h>
#include <qpainter.h>
#include <qvaluevector.h>
#include <qrect.h>

#include <qcolor.h>
#include <kcommand.h>

#include "kis_color.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_paint_device_impl.h"
#include "kis_point.h"
#include "kis_matrix.h"
#include "kis_progress_subject.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_pixel.h"
#include "kis_pattern.h"
#include <koffice_export.h>

// XXX: Filling should set dirty rect.
class KRITACORE_EXPORT KisFillPainter : public KisPainter
{

    typedef KisPainter super;

public:

        KisFillPainter();
        KisFillPainter(KisPaintDeviceImplSP device);


    /**
         * Fill a rectangle with black transparent pixels (0, 0, 0, 0 for RGBA).
     */
        void eraseRect(Q_INT32 x1, Q_INT32 y1, Q_INT32 w, Q_INT32 h);
        void eraseRect(const QRect& rc);

    /**
         * Fill a rectangle with a certain color.
     */
        void fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KisColor& c);
        void fillRect(const QRect& rc, const KisColor& c);

    /**
         * Fill a rectangle with a certain color and opacity.
     */
        void fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KisColor& c, QUANTUM opacity);
        void fillRect(const QRect& rc, const KisColor& c, QUANTUM opacity);

    /**
     * Fill a rectangle with a certain pattern. The pattern is repeated if it does not fit the
     * entire rectangle.
     */
    void fillRect(Q_INT32 x1, Q_INT32 y1, Q_INT32 w, Q_INT32 h, KisPattern * pattern);
    void fillRect(const QRect& rc, KisPattern * pattern);

    /**
     * Fills the enclosed area around the point with the set color. If there is a
     * selection, the whole selection is filled
     **/
    void fillColor(int startX, int startY);

    /**
     * Fills the enclosed area around the point with the set pattern. If there is a
     * selection, the whole selection is filled
     **/
    void fillPattern(int startX, int startY);
    
    /**
     * Returns a selection mask for the floodfill starting at the specified position.
     **/
    KisSelectionSP createFloodSelection(int startX, int startY);

    void setFillThreshold(int threshold);
    
    /** Sets the width of the layer */
    void setWidth(int w) { m_width = w; }

    /** Sets the height of the layer */
    void setHeight(int h) { m_height = h; }

    /** If sample merged is set to true, the paint device will get the bounds of the
     * floodfill from the complete image instead of the layer */

    bool sampleMerged() { return m_sampleMerged; }
    void setSampleMerged(bool set) { m_sampleMerged = set; }

private:
    // for floodfill
    /**
     * calculates the difference between 2 pixel values. Returns a value between 0 and
     * 255 (actually should be MIN_SELECTED to MAX_SELECTED?). Only 0 and 255 are
     * returned when anti-aliasing is off
     **/
    void genericFillStart(int startX, int startY);
    void genericFillEnd(KisPaintDeviceImplSP filled);

    KisSelectionSP m_selection;

    int m_threshold;
    int m_size;
    int m_width, m_height;
    QRect m_rect;
    bool m_sampleMerged;
};


inline
void KisFillPainter::fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KisColor& c)
{
        fillRect(x, y, w, h, c, OPACITY_OPAQUE);
}

inline
void KisFillPainter::fillRect(const QRect& rc, const KisColor& c)
{
        fillRect(rc.x(), rc.y(), rc.width(), rc.height(), c, OPACITY_OPAQUE);
}

inline
void KisFillPainter::eraseRect(Q_INT32 x1, Q_INT32 y1, Q_INT32 w, Q_INT32 h)
{
    KisColor c(Qt::black);
        fillRect(x1, y1, w, h, c, OPACITY_TRANSPARENT);
}

inline
void KisFillPainter::eraseRect(const QRect& rc)
{
    KisColor c(Qt::black);
        fillRect(rc.x(), rc.y(), rc.width(), rc.height(), c, OPACITY_TRANSPARENT);
}

inline
void KisFillPainter::fillRect(const QRect& rc, const KisColor& c, QUANTUM opacity)
{
        fillRect(rc.x(), rc.y(), rc.width(), rc.height(), c, opacity);
}

inline
void KisFillPainter::fillRect(const QRect& rc, KisPattern *pattern)
{
    fillRect(rc.x(), rc.y(), rc.width(), rc.height(), pattern);
}

inline
void KisFillPainter::setFillThreshold(int threshold)
{
    m_threshold = threshold;
}


#endif //KIS_FILL_PAINTER_H_
