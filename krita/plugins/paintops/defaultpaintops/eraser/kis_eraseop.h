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

#ifndef KIS_ERASEOP_H_
#define KIS_ERASEOP_H_

#include "kis_brush_based_paintop.h"
#include <klocale.h>
#include <QDialog>
#include <KoColorSpace.h>

#include <kis_paintop_factory.h>
#include <kis_paintop_settings.h>
#include <kis_paintop_options_widget.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>

class KisBrushOptionWidget;
class KisPaintActionTypeOption;
class KisEraseOpSettings;
class KisEraseOpSettingsWidget;

class QWidget;
class QPointF;
class KisPainter;


class KisEraseOp : public KisBrushBasedPaintOp
{

public:

    KisEraseOp(const KisEraseOpSettings *settings, KisPainter * painter, KisImageWSP image);
    virtual ~KisEraseOp();

    void paintAt(const KisPaintInformation& info);

private:

    const KisEraseOpSettings * settings;
    KisPressureOpacityOption m_opacityOption;
    KisPressureSizeOption m_sizeOption;
};

#endif // KIS_ERASEOP_H_
