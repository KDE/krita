/*
 *  Copyright (c) 2008-2009 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_MASK_GENERATOR_H_
#define _KIS_MASK_GENERATOR_H_

#include "krita_export.h"

class QDomElement;
class QDomDocument;

/**
 * This is the base class for mask shapes
 * You should subclass it if you want to create a new
 * shape.
 */
class KRITAIMAGE_EXPORT KisMaskGenerator
{
public:
    enum Type {
        CIRCLE, RECTANGLE
    };
public:

    KDE_DEPRECATED KisMaskGenerator(qreal width, qreal height, qreal fh, qreal fv, Type type);
    /**
     * This function creates an auto brush shape with the following value :
     * @param w width
     * @param h height
     * @param fh horizontal fade (fh \< w / 2 )
     * @param fv vertical fade (fv \< h / 2 )
     */
    KisMaskGenerator(qreal radius, qreal ratio, qreal fh, qreal fv, int spikes, Type type);

    virtual ~KisMaskGenerator();

private:

    void init();

public:
    /**
     * @return the alpha value at the position (x,y)
     */
    virtual quint8 valueAt(qreal x, qreal y) const = 0;

    quint8 interpolatedValueAt(qreal x, qreal y);

    virtual void toXML(QDomDocument& , QDomElement&) const;

    /**
     * Unserialise a \ref KisMaskGenerator
     */
    static KisMaskGenerator* fromXML(const QDomElement&);

    qreal width() const;

    qreal height() const;

    qreal radius() const;
    qreal ratio() const;
    qreal horizontalFade() const;
    qreal verticalFade() const;
    int spikes() const;
    Type type() const;
protected:
    struct Private {
        qreal m_radius, m_ratio;
        qreal m_fh, m_fv;
        int m_spikes;
        qreal cs, ss;
        bool m_empty;
        Type type;
    };

    Private* const d;
};

#endif
