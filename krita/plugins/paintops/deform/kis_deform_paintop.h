/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#ifndef KIS_DEFORMPAINTOP_H_
#define KIS_DEFORMPAINTOP_H_

#include <klocale.h>
#include <kis_paintop.h>
#include <kis_types.h>

#include "deform_brush.h"
#include "kis_deform_paintop_settings.h"

class QPointF;
class KisPainter;

class KisDeformPaintOp : public KisPaintOp
{

public:
    KisDeformPaintOp(const KisDeformPaintOpSettings *settings, KisPainter * painter, KisImageSP image);
    virtual ~KisDeformPaintOp();

    // Do we want to spray even when no movement?
    virtual bool incremental() const {
        return m_useMovementPaint;
    }

    void paintAt(const KisPaintInformation& info);
    double spacing(double & xSpacing, double & ySpacing, double pressure1, double pressure2) const;


private:
    bool m_useMovementPaint;

    KisPaintDeviceSP m_dab;
    KisPaintDeviceSP m_dev;

    DeformBrush m_deformBrush;
    qreal m_xSpacing;
    qreal m_ySpacing;
    qreal m_spacing;

};

#endif // KIS_DEFORMPAINTOP_H_
