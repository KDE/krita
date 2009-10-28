/*
 * Copyright (c) 2009 Lukáš Tvrdý (lukast.dev@gmail.com)
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
#include <kis_random_sub_accessor.h>


#include <KoColor.h>

#ifdef BENCHMARK
    #include <QTime>
#endif
#include <KoColorSpace.h>


KisGridPaintOp::KisGridPaintOp(const KisGridPaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
    : KisPaintOp( painter )
    , m_settings( settings )
    , m_image ( image )
{

    m_xSpacing = settings->gridWidth() * settings->scale();
    m_ySpacing = settings->gridHeight()* settings->scale();
    m_spacing = m_xSpacing;

    m_dab = new KisPaintDevice( painter->device()->colorSpace() );
    m_painter = new KisPainter(m_dab);
    m_painter->setPaintColor( painter->paintColor() );
    m_painter->setFillStyle(KisPainter::FillStyleForegroundColor);
    m_pixelSize = settings->node()->paintDevice()->colorSpace()->pixelSize();
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

    int gridWidth = m_settings->gridWidth() * m_settings->scale();
    int gridHeight = m_settings->gridHeight() * m_settings->scale();

    int divide;
    if (m_settings->pressureDivision()){
        divide = m_settings->divisionLevel() * info.pressure();
    }else{
        divide = m_settings->divisionLevel();
    }
    divide = qRound(m_settings->scale() * divide);

    int posX = qRound( info.pos().x() );
    int posY = qRound( info.pos().y() );

    QPoint dabPosition( posX - posX % gridWidth, posY - posY % gridHeight );
    QPoint dabRightBottom(dabPosition.x() + gridWidth, dabPosition.y() + gridHeight);

    divide = qMax(1, divide);
    qreal yStep = gridHeight / (qreal)divide;
    qreal xStep = gridWidth / (qreal)divide;

    KisRandomSubAccessorPixel acc = m_settings->node()->paintDevice()->createRandomSubAccessor();
    
    QRectF tile;
    KoColor color( painter()->paintColor() );
   
    qreal vertBorder = m_settings->vertBorder(); 
    qreal horzBorder = m_settings->horizBorder();
    if (m_settings->jitterBorder()){
        if (vertBorder == horzBorder){
            vertBorder = horzBorder = vertBorder * drand48();
        }else{
            vertBorder *= drand48();
            horzBorder *= drand48();
        }
    }
    
    bool shouldColor = true;
    // fill the tile
    if (m_settings->fillBackground()){
        m_dab->fill(dabPosition.x(), dabPosition.y(), gridWidth, gridHeight, painter()->backgroundColor().data());
    }
    
    for (int y = 0; y < divide; y++){
        for (int x = 0; x < divide; x++){
            // determine the tile size
            tile = QRectF(dabPosition.x() + x*xStep,dabPosition.y() + y*yStep, xStep, yStep);
            tile.adjust(vertBorder,horzBorder,-vertBorder,-horzBorder);
            tile = tile.normalized();

            // do color transformation
            if (shouldColor){
                if (m_settings->sampleInput()){
                    acc.moveTo(tile.center().x(), tile.center().y());
                    acc.sampledRawData( color.data() );
                }else{
                    memcpy(color.data(),painter()->paintColor().data(), m_pixelSize);
                }

                // mix the color with background color
                if (m_settings->mixBgColor())
                {       
                    KoMixColorsOp * mixOp = source()->colorSpace()->mixColorsOp();

                    const quint8 *colors[2];
                    colors[0] = color.data();
                    colors[1] = painter()->backgroundColor().data();

                    qint16 colorWeights[2];
                    int MAX_16BIT = 255;
                    qreal blend = info.pressure();

                    colorWeights[0] = static_cast<quint16>( blend * MAX_16BIT); 
                    colorWeights[1] = static_cast<quint16>( (1.0 - blend) * MAX_16BIT); 
                    mixOp->mixColors(colors, colorWeights, 2, color.data() );
                }

                if (m_settings->useRandomHSV()){
                    QHash<QString, QVariant> params;
                    params["h"] = (m_settings->hue() / 180.0) * drand48();
                    params["s"] = (m_settings->saturation() / 100.0) * drand48();
                    params["v"] = (m_settings->value() / 100.0) * drand48();

                    KoColorTransformation* transfo;
                    transfo = m_dab->colorSpace()->createColorTransformation("hsv_adjustment", params);
                    transfo->transform(color.data(), color.data() , 1);
                }
                
                if (m_settings->useRandomOpacity()){
                    quint8 alpha = qRound(drand48() * OPACITY_OPAQUE);
                    color.setOpacity( alpha );
                    m_painter->setOpacity( alpha );
                }

                if ( !m_settings->colorPerParticle() ){
                    shouldColor = false;
                }
                
                m_painter->setPaintColor(color);
            }

            // paint some element
            switch (m_settings->shape()){
                case 0:
                {
                            m_painter->paintEllipse( tile ); 
                            break;
                }
                case 1: 
                {
                            // anti-aliased version
                            //m_painter->paintRect(tile);
                            m_dab->fill(tile.topLeft().x(), tile.topLeft().y(), tile.width(), tile.height(), color.data());
                            break;
                }
                case 2:
                {
                            m_painter->drawDDALine(tile.topRight(), tile.bottomLeft());
                            break;
                }
                case 3:
                {
                            m_painter->drawLine( tile.topRight(), tile.bottomLeft() );
                            break;
                }
                case 4:
                {
                            m_painter->drawThickLine(tile.topRight(), tile.bottomLeft() , 1,10);
                            break;
                }
                default:
                {
                            kDebug() << " implement or exclude from GUI ";
                            break;
                }
            }
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