/*
 *  Copyright (c) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_experiment_paintop.h"
#include "kis_experiment_paintop_settings.h"

#include <cmath>

#include <QRect>
#include <QColor>
#include <QList>

#include <KoColor.h>
#include <KoColorSpace.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <kis_paintop.h>
#include <kis_selection.h>
#include <kis_random_accessor.h>
#include <KoCompositeOp.h>

#include <kis_experimentop_option.h>

KisExperimentPaintOp::KisExperimentPaintOp(const KisExperimentPaintOpSettings *settings, KisPainter * painter, KisImageSP image)
    : KisPaintOp( painter )
    , m_settings( settings )
    , m_image ( image )
{
    m_isFirst = true;
    m_positions.clear();

    m_startSize = settings->getDouble(EXPERIMENT_START_SIZE);
    m_endSize = settings->getDouble(EXPERIMENT_END_SIZE);

    // spacing 
    qreal average = qMax(m_startSize, m_endSize ) - qMin(m_startSize, m_endSize) * 0.5;
    if ( average * 0.5 > 1)
    {
        m_ySpacing = m_xSpacing = average * 0.5 * settings->getDouble(EXPERIMENT_SPACING);
    } else
    {
        m_ySpacing = m_xSpacing = 1.0;
    }
    m_spacing = m_xSpacing;

    
    
    if ( !settings->node() ){
        m_dev = 0;
    }else{
        m_dev = settings->node()->paintDevice();
    }
    
    m_size = 1;
    m_rotationOption.readOptionSetting(settings);
    m_sizeOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
}

KisExperimentPaintOp::~KisExperimentPaintOp()
{
}

double KisExperimentPaintOp::spacing(double & xSpacing, double & ySpacing, double pressure1, double pressure2) const {
        Q_UNUSED(pressure1);
        Q_UNUSED(pressure2);
        xSpacing = m_xSpacing;
        ySpacing = m_ySpacing;
        return m_spacing;
}

#define MEMORY
void KisExperimentPaintOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()) return;

    if (!m_dab) {
        m_dab = new KisPaintDevice(painter()->device()->colorSpace());
    }
    else {
        m_dab->clear();
    }

#ifdef MEMORY
    if (!m_oldData) {
        m_oldData = new KisPaintDevice(painter()->device()->colorSpace());
    }
    else {
        m_oldData->clear();
    }
#endif


    QRect rc; 
    QRect erasedRc;
   
    // delete the previous dab in final datasource which will be bitblt
    if (!m_isFirst){
        erasedRc = rc = m_previousDab;
        KisRectConstIteratorPixel srcIt = m_dev->createRectConstIterator(rc.left(), rc.top(), rc.width(), rc.height());
#ifdef MEMORY
        KisRectIteratorPixel dstIt = m_oldData->createRectIterator(rc.left(), rc.top(), rc.width(), rc.height());
#else
        KisRectIteratorPixel dstIt = m_dev->createRectIterator(rc.left(), rc.top(), rc.width(), rc.height());
#endif


        qint32 pixelSize = m_dev->pixelSize();
        for (;!srcIt.isDone(); ++srcIt, ++dstIt) {
            memcpy(dstIt.rawData(),srcIt.oldRawData(), pixelSize );
        }
#ifdef MEMORY
        QString compositeOpId = painter()->compositeOp()->id();
        painter()->setCompositeOp(COMPOSITE_CLEAR);
        painter()->bitBlt(rc.topLeft(), m_oldData, rc);
        painter()->setCompositeOp(compositeOpId);
#endif
    }

    m_painter = new KisPainter(m_dab);
    m_painter->setFillStyle(KisPainter::FillStyleForegroundColor);
    m_painter->setPaintColor(painter()->paintColor());

    
    m_positions.append(info);
    int size = m_positions.size();
    qreal part = 0.0;

    int diameter;
    qreal pad;
    QPointF pos;
    if (size == 1)
    {
        diameter = m_startSize;
        pad = 0.5 * diameter;
        pos = m_positions[0].pos();
        pos -= QPointF(pad,pad);
        m_painter->paintEllipse(pos.x(), pos.y(), diameter, diameter);
    }else
    {
        for (int i = 0; i < size; i++)
        {
            part = (i / (qreal)(size-1));

            diameter = (1.0 - part) * m_startSize + part * m_endSize; if (diameter < 1.0) continue;

            pad = 0.5 * diameter;

            pos = m_positions[i].pos();
            pos -= QPointF(pad,pad);

            m_painter->paintEllipse(pos.x(), pos.y(), diameter, diameter);
        }
    }
    
    
    
    // save the size of it so that we can delete it next time
    rc = m_previousDab = m_dab->extent();
    painter()->bitBlt(rc.topLeft(), m_dab, rc);
#ifndef MEMORY
    m_settings->node()->setDirty(erasedRc);
#endif
    m_isFirst = false;
}

