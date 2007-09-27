/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
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

#ifndef KIS_COMPLEX_COLOR_H_
#define KIS_COMPLEX_COLOR_H_

#include <QPoint>

#include "KoColor.h"
#include "KoColorSpace.h"

#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_painterly_overlay_colorspace.h"

/**
 * XXX: Add dox
 */
class KRITAIMAGE_EXPORT KisComplexColor : public KisPaintDevice {
    Q_OBJECT

public:

    KisComplexColor(KoColorSpace *colorSpace);
    KisComplexColor(KoColorSpace *colorSpace, const KoColor &kc);
    ~KisComplexColor();

    void fromKoColor(const KoColor &kc);
    KoColor simpleColor();

    KoColor defaultColor();
    quint8 * defaultProperty();
    void setDefaultColor(const KoColor &kc);
    void setDefaultProperty(const PropertyCell &pc);
    void setDefaultProperty(const int type, const float value);

    QSize size();
    void setSize(const QSize &size);

    int left();
    int top();
    int right();
    int bottom();

    quint8 *rawData(int x, int y);
    quint8 *rawData(const QPoint &p);
    PropertyCell * property(int x, int y);
    PropertyCell * property(const QPoint &p);
    float scaling();
    QPoint center();
    void center(int *x, int *y);

    void setColor(int x, int y, const KoColor &kc);
    void setColor(const QPoint &p, const KoColor &kc);
    void setProperty(int x, int y, const PropertyCell &pc);
    void setProperty(const QPoint &p, const PropertyCell &pc);
    void setProperty(int x, int y, const int t, const float v);
    void setProperty(const QPoint &p, const int t, const float v);
    float setScaling(float s);
    QPoint setCenter(int x, int y);
    QPoint setCenter(QPoint p);

    KisPaintDeviceSP dab(int w, int h);

private:

    void scale(int *x, int *y);
    void scale(QPoint *p);

    void absolute(int *x, int *y);
    void absolute(QPoint *p);

    int absLeft();
    int absTop();
    int absRight();
    int absBottom();

    void translate();

    float m_scaling;
    QPoint m_center;
    QPoint m_offset;
    QSize m_size;

    KoColor m_defaultColor;
    quint8 *m_defaultProperty;

};

typedef KisSharedPtr<KisComplexColor> KisComplexColorSP;

#endif // KIS_COMPLEX_COLOR_H_
