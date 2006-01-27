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

#include <qrect.h>

#include "kis_meta_registry.h"
#include "kis_color.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_painter.h"
#include "kis_types.h"
#include <koffice_export.h>

class KisPattern;

// XXX: Filling should set dirty rect.
/**
 * This painter can be used to fill paint devices in different ways. This can also be used
 * for flood filling related operations.
 */
class KRITACORE_EXPORT KisFillPainter : public KisPainter
{

    typedef KisPainter super;

public:

    /**
     * Construct an empty painter. Use the begin(KisPaintDeviceSP) method to attach
     * to a paint device
     */
    KisFillPainter();
    /**
     * Start painting on the specified paint device
     */
    KisFillPainter(KisPaintDeviceSP device);

    /**
     * Fill a rectangle with black transparent pixels (0, 0, 0, 0 for RGBA).
     */
    void eraseRect(Q_INT32 x1, Q_INT32 y1, Q_INT32 w, Q_INT32 h);
    /**
     * Overloaded version of the above function.
     */
    void eraseRect(const QRect& rc);

    /**
     * Fill a rectangle with a certain color.
     */
    void fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KisColor& c);
    /**
     * Overloaded version of the above function.
     */
    void fillRect(const QRect& rc, const KisColor& c);

    /**
     * Fill a rectangle with a certain color and opacity.
     */
    void fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KisColor& c, Q_UINT8 opacity);
    /**
     * Overloaded version of the above function.
     */
    void fillRect(const QRect& rc, const KisColor& c, Q_UINT8 opacity);

    /**
     * Fill a rectangle with a certain pattern. The pattern is repeated if it does not fit the
     * entire rectangle.
     */
    void fillRect(Q_INT32 x1, Q_INT32 y1, Q_INT32 w, Q_INT32 h, KisPattern * pattern);
    /**
     * Overloaded version of the above function.
     */
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

    /**
     * Set the threshold for floodfill. The range is 0-255: 0 means the fill will only
     * fill parts that are the exact same color, 255 means anything will be filled
     */
    void setFillThreshold(int threshold);
    /** Returns the fill threshold, see setFillThreshold for details */
    int fillThreshold() const { return m_threshold; }

    /** Sets the width of the layer */
    void setWidth(int w) { m_width = w; }

    /** Sets the height of the layer */
    void setHeight(int h) { m_height = h; }

    /** If sample merged is set to true, the paint device will get the bounds of the
     * floodfill from the complete image instead of the layer */
    bool sampleMerged() const { return m_sampleMerged; }
    /** Set sample merged. See sampleMerged() for details */
    void setSampleMerged(bool set) { m_sampleMerged = set; }

    /** If true, floodfill doesn't fill outside the selected area of a layer */
    bool careForSelection() const { return m_careForSelection; }
    /** Set caring for selection. See careForSelection for details */
    void setCareForSelection(bool set) { m_careForSelection = set; }

    /**
     * If true, the floodfill will be fuzzy. This means that the 'value' of selectedness
     * will depend on the difference between the sampled color and the color at the current
     * position.
     */
    bool fuzzyFill() const { return m_fuzzy; }
    /** Sets the fuzzyfill parameter. See fuzzyFill for details */
    void setFuzzyFill(bool set) { m_fuzzy = set; }

private:
    // for floodfill
    void genericFillStart(int startX, int startY);
    void genericFillEnd(KisPaintDeviceSP filled);

    KisSelectionSP m_selection;

    int m_threshold;
    int m_size;
    int m_width, m_height;
    QRect m_rect;
    bool m_sampleMerged;
    bool m_careForSelection;
    bool m_fuzzy;
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
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
    KisColor c(Qt::black, cs);
    fillRect(x1, y1, w, h, c, OPACITY_TRANSPARENT);
}

inline
void KisFillPainter::eraseRect(const QRect& rc)
{
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
    KisColor c(Qt::black, cs);
    fillRect(rc.x(), rc.y(), rc.width(), rc.height(), c, OPACITY_TRANSPARENT);
}

inline
void KisFillPainter::fillRect(const QRect& rc, const KisColor& c, Q_UINT8 opacity)
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
