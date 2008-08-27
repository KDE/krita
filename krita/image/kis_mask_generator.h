/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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
 * This is the base class of auto brush shape. You should subclass it if you want to create new shape.
 */
class KRITAIMAGE_EXPORT KisMaskGenerator
{
public:
    virtual ~KisMaskGenerator() {}
    /**
     * This function creates an auto brush shape with the following value :
     * @param w width
     * @param h height
     * @param fh horizontal fade (fh \< w / 2 )
     * @param fv vertical fade (fv \< h / 2 )
     */
    KisMaskGenerator(double w, double h, double fh, double fv) : m_w(w), m_h(h), m_fh(fh), m_fv(fv) { }
    /**
     * @return the alpha value at the position (x,y)
     */
    virtual quint8 valueAt(double x, double y) = 0;
    quint8 interpolatedValueAt(double x, double y);
    virtual void toXML(QDomDocument& , QDomElement&) const;
    /**
     * Unserialise a \ref KisMaskGenerator
     */
    static KisMaskGenerator* fromXML(const QDomElement&);
    double width() const {
        return m_w;
    }
    double height() const {
        return m_h;
    }
protected:
    double m_w, m_h;
    double m_fh, m_fv;
};

/**
 * This class allow to create circular shape.
 */
class KRITAIMAGE_EXPORT KisCircleMaskGenerator : public KisMaskGenerator
{
public:
    virtual ~KisCircleMaskGenerator() {}
    KisCircleMaskGenerator(double w, double h, double fh, double fv);
    virtual quint8 valueAt(double x, double y);
    virtual void toXML(QDomDocument& , QDomElement&) const;
private:
    double norme(double a, double b) {
        return a*a + b * b;
    }
private:
    double m_xcenter, m_ycenter;
    double m_xcoef, m_ycoef;
    double m_xfadecoef, m_yfadecoef;
};

/**
 * This class allow to create rectangular shape.
 */
class KRITAIMAGE_EXPORT KisRectangleMaskGenerator : public KisMaskGenerator
{
public:
    virtual ~KisRectangleMaskGenerator() {}
    KisRectangleMaskGenerator(double w, double h, double fh, double fv);
    virtual quint8 valueAt(double x, double y);
    virtual void toXML(QDomDocument& , QDomElement&) const;
private:
    double m_xcenter, m_ycenter, m_c;
};


#endif
