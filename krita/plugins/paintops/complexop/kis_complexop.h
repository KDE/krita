/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2008 Emanuele Tamponi <emanuele@valinor.it>
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

#ifndef KIS_COMPLEXOP_H_
#define KIS_COMPLEXOP_H_

#include "kis_brush_based_paintop.h"
#include <kis_pressure_darken_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>

class KisBrushOptionWidget;
class KisPaintActionTypeOption;
class KisComplexOpSettings;
class KisComplexOpSettingsWidget;

class QWidget;
class QPointF;
class KisPainter;
class KisCurveWidget;

class KisComplexOp : public KisBrushBasedPaintOp
{

public:

    KisComplexOp(const KisComplexOpSettings *settings, KisPainter * painter, KisImageWSP image);
    virtual ~KisComplexOp();

    void paintAt(const KisPaintInformation& info);

private:

    const KisComplexOpSettings * settings;
    KisPressureOpacityOption m_opacityOption;
    KisPressureDarkenOption m_darkenOption;
    KisPressureSizeOption m_sizeOption;
};

#endif // KIS_COMPLEXOP_H_
