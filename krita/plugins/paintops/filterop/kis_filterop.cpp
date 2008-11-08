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

#include "kis_filterop.h"

#include <string.h>

#include <QRect>
#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include <QDomElement>
#include <QHBoxLayout>
#include <qtoolbutton.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <KoColorTransformation.h>
#include <KoColor.h>
#include <KoInputDevice.h>

#include <widgets/kcurve.h>
#include <kis_brush.h>
#include <kis_datamanager.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_properties_configuration.h>
#include <kis_selection.h>
#include <kis_brush_option.h>
#include <kis_paintop_options_widget.h>
#include <kis_pressure_size_option.h>
#include <kis_filter_option.h>
#include <kis_filterop_settings.h>
#include <kis_filterop_settings_widget.h>


KisFilterOp::KisFilterOp(const KisFilterOpSettings *settings, KisPainter *painter)
        : KisBrushBasedPaintOp(painter)
        , settings(settings)
{
    Q_ASSERT(settings);
    Q_ASSERT(painter);
    Q_ASSERT(settings->m_optionsWidget->m_brushOption);
    m_brush = settings->m_optionsWidget->m_brushOption->brush();
    Q_ASSERT( m_brush );
}

KisFilterOp::~KisFilterOp()
{
}

void KisFilterOp::paintAt(const KisPaintInformation& info)
{
}

double KisFilterOp::paintLine(const KisPaintInformation &pi1,
                           const KisPaintInformation &pi2,
                           double savedDist)
{
    KisPaintInformation adjustedInfo1(pi1);
    KisPaintInformation adjustedInfo2(pi2);
    if (!settings->m_optionsWidget->m_sizeOption->isChecked()) {
        adjustedInfo1.setPressure(PRESSURE_DEFAULT);
        adjustedInfo2.setPressure(PRESSURE_DEFAULT);
    }
    return KisPaintOp::paintLine(adjustedInfo1, adjustedInfo2, savedDist);
}
