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

#ifndef _KIS_CIRCLE_MASK_GENERATOR_H_
#define _KIS_CIRCLE_MASK_GENERATOR_H_

#include "krita_export.h"

#include "kis_mask_generator.h"

class QDomElement;
class QDomDocument;

/**
 * Create, serialize and deserialize an elliptical 8-bit mask.
 */
class KRITAIMAGE_EXPORT KisCircleMaskGenerator : public KisMaskGenerator
{

public:

    KDE_DEPRECATED KisCircleMaskGenerator(double w, double h, double fh, double fv);
    KisCircleMaskGenerator(double radius, double ratio, double fh, double fv, int spikes);
    virtual ~KisCircleMaskGenerator();

    virtual quint8 valueAt(double x, double y) const;

    virtual void toXML(QDomDocument& , QDomElement&) const;

private:

    double norme(double a, double b) const {
        return a*a + b * b;
    }

private:
    struct Private;
    Private* const d;
};

#endif
