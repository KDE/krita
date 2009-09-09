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

    KDE_DEPRECATED KisMaskGenerator(double width, double height, double fh, double fv);
    /**
     * This function creates an auto brush shape with the following value :
     * @param w width
     * @param h height
     * @param fh horizontal fade (fh \< w / 2 )
     * @param fv vertical fade (fv \< h / 2 )
     */
    KisMaskGenerator(double radius, double ratio, double fh, double fv, int spikes);

    virtual ~KisMaskGenerator();

private:

    void init();

public:
    /**
     * @return the alpha value at the position (x,y)
     */
    virtual quint8 valueAt(double x, double y) const = 0;

    quint8 interpolatedValueAt(double x, double y);

    virtual void toXML(QDomDocument& , QDomElement&) const;

    /**
     * Unserialise a \ref KisMaskGenerator
     */
    static KisMaskGenerator* fromXML(const QDomElement&);

    double width() const;

    double height() const;

protected:
    struct Private {
        double m_radius, m_ratio;
        double m_fh, m_fv;
        int m_spikes;
        double cs, ss;
        bool m_empty;
    };

    Private* const d;
};

#endif
