/*
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_deform_paintop.h"
#include "kis_deform_paintop_settings.h"

#include <cmath>

#include <QRect>

#include <kis_image.h>
#include <kis_debug.h>

#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_paintop.h"
#include "kis_selection.h"
#include "kis_random_accessor.h"

KisDeformPaintOp::KisDeformPaintOp(const KisDeformPaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
        : KisPaintOp(painter)
{
    Q_ASSERT(settings);
    m_deformBrush.setAction(settings->deformAction());
    m_deformBrush.setRadius(settings->radius());
    m_deformBrush.setDeformAmount(settings->deformAmount());
    m_deformBrush.setInterpolation(settings->bilinear());

    m_deformBrush.setCounter(1);
    m_useMovementPaint = settings->useMovementPaint();
    m_deformBrush.setUseCounter(settings->useCounter());
    m_deformBrush.setUseOldData(settings->useOldData());

    m_dev = source();

    if ((settings->radius()) > 1) {
        m_ySpacing = m_xSpacing = settings->radius() * settings->spacing();
    } else {
        m_ySpacing = m_xSpacing = 1.0;
    }
    m_spacing = m_xSpacing;

}

KisDeformPaintOp::~KisDeformPaintOp()
{
}

double KisDeformPaintOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()) return m_spacing;
    if (!m_dev) return m_spacing;
 
    if (!m_dab) {
        m_dab = new KisPaintDevice(painter()->device()->colorSpace());
    } else {
        m_dab->clear();
    }

    //write device, read device, position
    m_deformBrush.paint(m_dab, m_dev, info);

    QRect rc = m_dab->extent();
    painter()->bitBlt(rc.x(), rc.y(), m_dab, rc.x(), rc.y(), rc.width(), rc.height());
    return m_spacing;
}
