/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <qrect.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <Q3HBoxLayout>

#include <kdebug.h>

#include "kis_brush.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_layer.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_paintop.h"
#include "kis_input_device.h"
#include "kis_selection.h"
#include "kis_brushop.h"

KisPaintOp * KisBrushOpFactory::createOp(const KisPaintOpSettings *settings, KisPainter * painter)
{
    const KisBrushOpSettings *brushopSettings = dynamic_cast<const KisBrushOpSettings *>(settings);
    Q_ASSERT(settings == 0 || brushopSettings != 0);

    KisPaintOp * op = new KisBrushOp(brushopSettings, painter);
    Q_CHECK_PTR(op);
    return op;
}

KisBrushOpSettings::KisBrushOpSettings(QWidget *parent)
    : super(parent)
{
    m_optionsWidget = new QWidget(parent, "brush option widget");
    Q3HBoxLayout * l = new Q3HBoxLayout(m_optionsWidget);
    l->setAutoAdd(true);
    m_pressureVariation = new QLabel(i18n("Pressure variation: "), m_optionsWidget);
    m_size =  new QCheckBox(i18n("size"), m_optionsWidget);
    m_size->setChecked(true);
    m_opacity = new QCheckBox(i18n("opacity"), m_optionsWidget);
    m_darken = new QCheckBox(i18n("darken"), m_optionsWidget);
}

bool KisBrushOpSettings::varySize() const
{
    return m_size->isChecked();
}

bool KisBrushOpSettings::varyOpacity() const
{
    return m_opacity->isChecked();
}

bool KisBrushOpSettings::varyDarken() const
{
    return m_darken->isChecked();
}

KisPaintOpSettings* KisBrushOpFactory::settings(QWidget * parent, const KisInputDevice& inputDevice)
{
    if (inputDevice == KisInputDevice::mouse()) {
        // No options for mouse, only tablet devices
        return 0;
    } else {
        return new KisBrushOpSettings(parent);
    }
}

KisBrushOp::KisBrushOp(const KisBrushOpSettings *settings, KisPainter *painter)
    : super(painter)
    , m_pressureSize(true)
    , m_pressureOpacity(false)
    , m_pressureDarken(false)
{
    if (settings != 0) {
        m_pressureSize = settings->varySize();
        m_pressureOpacity = settings->varyOpacity();
        m_pressureDarken = settings->varyDarken();
    }
}

KisBrushOp::~KisBrushOp()
{
}

void KisBrushOp::paintAt(const KisPoint &pos, const KisPaintInformation& info)
{
    KisPaintInformation adjustedInfo(info);
    if (!m_pressureSize)
        adjustedInfo.pressure = PRESSURE_DEFAULT;


    // Painting should be implemented according to the following algorithm:
    // retrieve brush
    // if brush == mask
    //          retrieve mask
    // else if brush == image
    //          retrieve image
    // subsample (mask | image) for position -- pos should be double!
    // apply filters to mask (colour | gradient | pattern | etc.
    // composite filtered mask into temporary layer
    // composite temporary layer into target layer
    // @see: doc/brush.txt

    if (!m_painter->device()) return;

    KisBrush *brush = m_painter->brush();
    
    Q_ASSERT(brush);
    if (!brush) return;
    if (! brush->canPaintFor(adjustedInfo) )
        return;
    
    KisPaintDeviceSP device = m_painter->device();

    KisPoint hotSpot = brush->hotSpot(adjustedInfo);
    KisPoint pt = pos - hotSpot;

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    qint32 x;
    double xFraction;
    qint32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    KisPaintDeviceSP dab = 0;

    quint8 origOpacity = m_painter->opacity();
    KisColor origColor = m_painter->paintColor();

    if (m_pressureOpacity)
        m_painter->setOpacity((qint8)(origOpacity * info.pressure));

    if (m_pressureDarken) {
        KisColor darkened = origColor;
        // Darken docs aren't really clear about what exactly the amount param can have as value...
        darkened.colorSpace()->darken(origColor.data(), darkened.data(),
            (qint32)(255  - 75 * info.pressure), false, 0.0, 1);
        m_painter->setPaintColor(darkened);
    }

    if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        dab = brush->image(device->colorSpace(), adjustedInfo, xFraction, yFraction);
    }
    else {
        KisAlphaMaskSP mask = brush->mask(adjustedInfo, xFraction, yFraction);
        dab = computeDab(mask);
    }

    m_painter->setPressure(adjustedInfo.pressure);

    QRect dabRect = QRect(0, 0, brush->maskWidth(adjustedInfo),
                          brush->maskHeight(adjustedInfo));
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());

    KisImage * image = device->image();
    
    if (image != 0) {
        dstRect &= image->bounds();
    }
    
    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    qint32 sx = dstRect.x() - x;
    qint32 sy = dstRect.y() - y;
    qint32 sw = dstRect.width();
    qint32 sh = dstRect.height();
    
    if (m_source->hasSelection()) {
        m_painter->bltSelection(dstRect.x(), dstRect.y(), m_painter->compositeOp(), dab.data(),
                                m_source->selection(), m_painter->opacity(), sx, sy, sw, sh);
    }
    else {
        m_painter->bitBlt(dstRect.x(), dstRect.y(), m_painter->compositeOp(), dab.data(), m_painter->opacity(), sx, sy, sw, sh);
    }
    m_painter->addDirtyRect(dstRect);

    m_painter->setOpacity(origOpacity);
    m_painter->setPaintColor(origColor);
}
