/*
 *  Copyright (c) 2008-2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_grid_paintop.h"
#include "kis_grid_paintop_settings.h"

#include <cmath>

#include <QRect>

#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <kis_paintop.h>
#include <kis_paint_information.h>

#include <KoColor.h>

#ifdef BENCHMARK
    #include <QTime>
#endif


KisGridPaintOp::KisGridPaintOp(const KisGridPaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
    : KisPaintOp( painter )
    , m_settings( settings )
    , m_image ( image )
{

    m_xSpacing = settings->gridWidth();
    m_ySpacing = settings->gridHeight();
    m_spacing = m_xSpacing;

    m_dab = new KisPaintDevice( painter->device()->colorSpace() );
    m_painter = new KisPainter(m_dab);
    m_painter->setPaintColor( painter->paintColor() );
    m_painter->setFillStyle(KisPainter::FillStyleForegroundColor);
    
#ifdef BENCHMARK
    m_count = m_total = 0;
#endif
    
}

KisGridPaintOp::~KisGridPaintOp()
{
}

double KisGridPaintOp::spacing(double & xSpacing, double & ySpacing, double pressure1, double pressure2) const {
        Q_UNUSED(pressure1);
        Q_UNUSED(pressure2);
        xSpacing = m_xSpacing;
        ySpacing = m_ySpacing;

        return m_spacing;
}


void KisGridPaintOp::paintAt(const KisPaintInformation& info)
{
#ifdef BENCHMARK
    QTime time;
    time.start();
#endif
if (!painter()) return;
    m_dab->clear();

    int gridWidth = m_settings->gridWidth();
    int gridHeight = m_settings->gridHeight();

    int divide;
    if (m_settings->pressureDivision()){
        divide = m_settings->divisionLevel() * info.pressure();
    }else{
        divide = m_settings->divisionLevel();
    }

    int posX = qRound( info.pos().x() );
    int posY = qRound( info.pos().y() );

    QPoint dabPosition( posX - posX % gridWidth, posY - posY % gridHeight );
    QPoint dabRightBottom(dabPosition.x() + gridWidth, dabPosition.y() + gridHeight);

    divide = qMax(1, divide);
    qreal yStep = gridHeight / (qreal)divide;
    qreal xStep = gridWidth / (qreal)divide;


    for (int y = 0; y < divide; y++){
        for (int x = 0; x < divide; x++){
            m_painter->paintEllipse(dabPosition.x() + x*xStep,dabPosition.y() + y*yStep, xStep, yStep );
        }
    }
    
    QRect rc = m_dab->extent();
    painter()->bitBlt(rc.topLeft(), m_dab, rc);

    
#ifdef BENCHMARK
    int msec = time.elapsed();
    kDebug() << msec << " ms/dab " << "[average: " << m_total / (qreal)m_count << "]";
    m_total += msec;
    m_count++;
#endif
}

