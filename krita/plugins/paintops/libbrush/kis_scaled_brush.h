/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_SCALED_BRUSH_H
#define KIS_SCALED_BRUSH_H

#include <QImage>

#include <kis_types.h>

#include "kis_qimage_mask.h"

class KisScaledBrush
{

public:

    KisScaledBrush();

    KisScaledBrush(KisQImagemaskSP scaledMask,
                   const QImage& scaledImage,
                   double scale, double xScale, double yScale);

    double scale() const {
        return m_scale;
    }
    double xScale() const {
        return m_xScale;
    }
    double yScale() const {
        return m_yScale;
    }
    KisQImagemaskSP mask() const {
        return m_mask;
    }
    QImage image() const {
        return m_image;
    }

private:
    KisQImagemaskSP m_mask;
    QImage m_image;
    double m_scale;
    double m_xScale;
    double m_yScale;
};

#endif
