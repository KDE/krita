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
#include <kis_iterator_ng.h>

static const QColor black(Qt::black);
static const QBrush brush(Qt::white);

KisExperimentPaintOp::KisExperimentPaintOp(const KisExperimentPaintOpSettings *settings, KisPainter * painter, KisImageSP image)
    : KisPaintOp( painter )
    , m_settings( settings )
{
    m_isFirst = true;
    
    m_rotationOption.readOptionSetting(settings);
    m_sizeOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_experimentOption.readOptionSetting(settings);

    m_displacement = (m_experimentOption.displacement * 0.01 * 14) + 1; // 1..15 [7 default according alchemy]
    m_multiplier = (m_experimentOption.speed * 0.01 * 35); // 0..35 [15 default according alchemy]
    m_smoothing = m_experimentOption.smoothing;
    
    m_path = QPainterPath();
    m_polygonMaskImage = QImage(256, 256, QImage::Format_ARGB32_Premultiplied);
    m_polygonDevice = new KisFixedPaintDevice(source()->colorSpace());
    
    m_currentLayerDevice = new KisPaintDevice(source()->colorSpace());
    m_currentLayerDevice->prepareClone(source());
    m_currentLayerDevice->makeCloneFromRough(source(), source()->extent());

}

KisExperimentPaintOp::~KisExperimentPaintOp()
{
}


void KisExperimentPaintOp::clearPreviousDab()
{
        qint32 pixelSize = m_currentLayerDevice->pixelSize();
        foreach (const QRect &previousDab, m_previousDabs){

            if (!previousDab.isEmpty()){
                KisRectIteratorSP dstIt = m_currentLayerDevice->createRectIteratorNG(previousDab.left(), previousDab.top(), previousDab.width(), previousDab.height());
                do {
                    memcpy(dstIt->rawData(),dstIt->oldRawData(), pixelSize );
                } while (dstIt->nextPixel());
                
            }
        }
        
        foreach (const QRect &previousDab, m_previousDabs){
            m_settings->node()->setDirty(previousDab);
        }
}

void KisExperimentPaintOp::clearPreviousDab2()
{
    if (m_previousDabs.size() == 0) return; 
    
    QRect result = m_previousDabs[0];
    if (result.isEmpty()) return;
    
    for (int i = 1; i < m_previousDabs.size(); i++){
        result = result.united(m_previousDabs.at(i));
    }
    
    const KoCompositeOp * orig = painter()->compositeOp();
    painter()->setCompositeOp(COMPOSITE_COPY);
    painter()->bitBlt(result.topLeft(),m_currentLayerDevice,result);
    painter()->setCompositeOp(orig);
}



KisDistanceInformation KisExperimentPaintOp::paintLine(const KisPaintInformation& pi1, const KisPaintInformation& pi2, const KisDistanceInformation& savedDist)
{
    KisDistanceInformation kdi(0,0);
    if (!painter()) return kdi;

    if (m_isFirst){
        m_path.moveTo(pi1.pos());
        addPosition(pi2.pos());
    }else{
        // pre-process
        if (m_experimentOption.isSpeedEnabled){
            int speed = getCursorSpeed(pi2.pos(), pi1.pos()) / 2;
            QPointF pos = getAngle(pi2.pos(), pi1.pos(), m_experimentOption.speed * m_multiplier);
            addPosition(pos);
        }else{
            addPosition(pi2.pos());
        }
        
        // post-process
        if (m_experimentOption.isDisplacementEnabled) {
            int speed = m_displacement - getCursorSpeed(pi2.pos(),pi1.pos());
            m_path = applyDisplace(m_path, speed);
        }
        
        // delete the previous dab in final datasource which will be bitblt
        clearPreviousDab2();        
        // render the new one
        quint8 origOpacity = m_opacityOption.apply(painter(), pi2);
        fillPainterPath(m_path);
        painter()->setOpacity(origOpacity);
    }

    m_isFirst = false;
    return kdi;
}

qreal KisExperimentPaintOp::paintAt(const KisPaintInformation& info)
{
    return 1.0;
}


QPainterPath KisExperimentPaintOp::applyDisplace(const QPainterPath& path, int speed)
{
    QPointF lastPoint = path.currentPosition();
    
    QPainterPath newPath;
    int count = path.elementCount();
    int curveElementCounter = 0;    
    QPointF ctrl1;
    QPointF ctrl2;
    QPointF endPoint;
    for (int i = 0; i < count; i++){
        QPainterPath::Element e = path.elementAt(i);
        switch(e.type){
            case QPainterPath::MoveToElement:{

                newPath.moveTo(getAngle(QPointF(e.x,e.y),lastPoint,speed));
                break;
            }
            case QPainterPath::LineToElement:{
                newPath.lineTo(getAngle(QPointF(e.x,e.y),lastPoint,speed));
                break;
            }
            case QPainterPath::CurveToElement:{
                curveElementCounter = 0; 
                endPoint = getAngle(QPointF(e.x,e.y),lastPoint,speed);
                break;
            }
            case QPainterPath::CurveToDataElement:{
                curveElementCounter++;
                
                if (curveElementCounter == 1){
                    ctrl1 = QPointF(e.x,e.y);
                }else if (curveElementCounter == 2){
                    ctrl2 = QPointF(e.x,e.y);
                    newPath.cubicTo(ctrl1,ctrl2,endPoint);
                }
                break;
            }
        }
    
    }// for
    
    return newPath;
}


void KisExperimentPaintOp::fillPainterPath(const QPainterPath& path)
{
        // anti-aliasing
        QRectF boundingRect = path.boundingRect();
        QRect fillRect;

        fillRect.setLeft((qint32)floor(boundingRect.left()));
        fillRect.setRight((qint32)ceil(boundingRect.right()));
        fillRect.setTop((qint32)floor(boundingRect.top()));
        fillRect.setBottom((qint32)ceil(boundingRect.bottom()));

        // Expand the rectangle to allow for anti-aliasing.
        fillRect.adjust(-1, -1, 1, 1);

        QSize fillRectSize = fillRect.size();
        if (m_polygonMaskImage.width() < fillRectSize.width() || m_polygonMaskImage.height() < fillRectSize.height()){
            m_polygonMaskImage = QImage(fillRectSize.width(), fillRectSize.height(), m_polygonMaskImage.format());
        }
        m_polygonMaskImage.fill( black.rgb() );
        
        // save to QImage
        QPainter pathPainter(&m_polygonMaskImage);
        pathPainter.setRenderHint(QPainter::Antialiasing, true);
        pathPainter.translate(-path.boundingRect().topLeft());
        pathPainter.translate(1,1);
        pathPainter.fillPath(path, brush);

        //m_polygonMaskImage.save("test.png");
        //painter.translate(-1,-1);
        //painter.translate(m_path.boundingRect().topLeft());
        
        //convert to device
        QRect polygonRect(0,0,fillRectSize.width(), fillRectSize.height());
        m_polygonDevice->setRect(polygonRect);
        if (m_polygonDevice->allocatedPixels() < fillRectSize.width() * fillRectSize.height()) {
            m_polygonDevice->initialize();
        }
        
        
        {
            int pixelSize = m_polygonDevice->pixelSize();
            int blockPixelCount = qMax(fillRectSize.width(), fillRectSize.height());
            int blockPixelSize = blockPixelCount * pixelSize;
            quint8 * pixelLine = new quint8[ blockPixelSize ];
            quint8 * pit = pixelLine;
            for (int i = 0; i < blockPixelCount; i++){
                memcpy(pit, painter()->paintColor().data(), pixelSize);
                pit += pixelSize;
            }
            
            
            quint8 * dabPointer = m_polygonDevice->data();
            int lines = qMin(fillRectSize.width(), fillRectSize.height());
            for (int y = 0; y < lines; y++){
                memcpy(dabPointer, pixelLine, blockPixelSize);
                dabPointer += blockPixelSize;
            }
            
            delete [] pixelLine;    
        }
        //m_polygonDevice->fill(0,0,fillRectSize.width(), fillRectSize.height(), painter()->paintColor().data());
        
        // much more faster m_polygonDevice->fill ^
        quint8 * data = m_polygonDevice->data();
        int rowSize = m_polygonDevice->pixelSize() * fillRectSize.width();
        const KoColorSpace * cs = source()->colorSpace();
        
        quint8 * alphaLine = new quint8[ fillRectSize.width() ]; // alpha has pixelSize == 1
        quint8 * it = alphaLine;
        
        quint8 pixelSize = m_polygonDevice->pixelSize();
        for (int y = 0; y < fillRectSize.height(); y++){
            QRgb * line = reinterpret_cast<QRgb*>(m_polygonMaskImage.scanLine(y));
            for (int x = 0; x < fillRectSize.width(); x++){
                *it = quint8(qRed(line[x]));
                it += 1;
            }
            
            cs->applyAlphaU8Mask( data, alphaLine, fillRectSize.width() );
            data += pixelSize * fillRectSize.width();
            it = alphaLine;
        }
        delete [] alphaLine;
        
        painter()->bltFixed(fillRect.topLeft(),m_polygonDevice, polygonRect);
        
        // save the area of the dab so that we can delete it next time
        m_previousDabs = regionsRenderMirrorMask(fillRect,m_polygonDevice);
        m_previousDabs.append( QRect(fillRect.topLeft(),polygonRect.size()) );
       
}



QPointF KisExperimentPaintOp::getAngle(const QPointF& p1, const QPointF& p2, double distance)
{
    double angle = atan2(p1.y() - p2.y(), p1.x() - p2.x());
    double x = p1.x() + (distance * cos(angle));
    double y = p1.y() + (distance * sin(angle));
    return QPointF(x,y);
}

void KisExperimentPaintOp::curveTo(QPainterPath& path, QPointF pos)
{
    QPointF pt = (path.currentPosition() + pos) * 0.5;
    path.quadTo(path.currentPosition(), pt);
}

void KisExperimentPaintOp::addPosition(const QPointF& pos)
{
    if (m_smoothing){
        curveTo(m_path,pos);
    }else{
        m_path.lineTo(pos);
    }
}
