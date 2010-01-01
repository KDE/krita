/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_BRUSHOP_H_
#define KIS_BRUSHOP_H_

#include "kis_brush_based_paintop.h"
#include <kis_pressure_darken_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_paint_action_type_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_mix_option.h>

class KisBrushOptionWidget;
class KisBrushOpSettings;
class KisBrushOpSettingsWidget;

class QWidget;
class QPointF;
class KisPainter;
class KisColorSource;


class KisBrushOp : public KisBrushBasedPaintOp
{

public:

    KisBrushOp(const KisBrushOpSettings *settings, KisPainter * painter, KisImageWSP image);
    virtual ~KisBrushOp();

    void paintAt(const KisPaintInformation& info);

private:
    KisColorSource* m_colorSource;
    const KisBrushOpSettings * settings;
    KisPressureSizeOption m_sizeOption;
    KisPressureOpacityOption m_opacityOption;
    KisPressureDarkenOption m_darkenOption;
    KisPressureRotationOption m_rotationOption;
    KisPressureMixOption m_mixOption;
};

#endif // KIS_BRUSHOP_H_
