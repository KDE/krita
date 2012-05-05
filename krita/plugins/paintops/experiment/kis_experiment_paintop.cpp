/*
 *  Copyright (c) 2010-2011 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include <kis_random_accessor_ng.h>
#include <KoCompositeOp.h>

#include <kis_experimentop_option.h>
#include <kis_transaction.h>
#include "kis_fixed_painter.h"

static const QColor BLACK(Qt::black);
static const QBrush KIS_WHITE_BRUSH(Qt::white);

KisExperimentPaintOp::KisExperimentPaintOp(const KisExperimentPaintOpSettings *settings, KisPainter * painter, KisImageSP image)
    : KisPaintOp( painter )
    , m_settings( settings )
{
    Q_UNUSED(image);
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

    m_fixedPainter = new KisFixedPainter(m_polygonDevice);
    m_fixedPainter->setPaintColor(painter->paintColor());

    m_currentLayerDevice = new KisPaintDevice(source()->colorSpace());
    m_currentLayerDevice->prepareClone(source());
    m_currentLayerDevice->makeCloneFromRough(source(), source()->extent());

    m_copyPainter = new KisPainter(m_currentLayerDevice);
    m_copyPainter->setOpacity(painter->opacity());
    m_copyPainter->setPaintColor(painter->paintColor());
    m_copyPainter->setCompositeOp(painter->compositeOp());
    m_copyPainter->setFillStyle(KisPainter::FillStyleForegroundColor);
}

KisExperimentPaintOp::~KisExperimentPaintOp()
{
    delete m_copyPainter;
    delete m_fixedPainter;
}


KisDistanceInformation KisExperimentPaintOp::paintLine(const KisPaintInformation& pi1, const KisPaintInformation& pi2, const KisDistanceInformation& /*savedDist*/)
{
    KisDistanceInformation kdi(0,0);
    if (!painter()) return kdi;

    // create the QPainterPath
    if (m_isFirst){
        m_path.moveTo(pi1.pos());
        addPosition(pi2.pos());
        //qDebug() << "Running the new version with KisTransaction.";
    }else{
        // pre-process
        if (m_experimentOption.isSpeedEnabled) {
            int speed = getCursorSpeed(pi2.pos(), pi1.pos()) / 2;
            QPointF pos = getAngle(pi2.pos(), pi1.pos(), speed * m_multiplier);
            addPosition(pos);
        }else{
            addPosition(pi2.pos());
        }

        // post-process
        if (m_experimentOption.isDisplacementEnabled) {
            int speed = m_displacement - getCursorSpeed(pi2.pos(),pi1.pos());
            m_path = applyDisplace(m_path, speed);
        }


        quint8 origOpacity = m_opacityOption.apply(m_copyPainter, pi2);

        QRect rc = m_fixedPainter->fillPainterPath(m_path);
        KisTransaction transaction("", m_currentLayerDevice);
        m_copyPainter->bltFixed(rc.topLeft().x(),rc.topLeft().y(), m_polygonDevice, 0,0,rc.width(), rc.height());
        painter()->copyMirrorInformation( m_copyPainter );
        QVector<QRect> rects = m_copyPainter->regionsRenderMirrorMask(rc, m_polygonDevice);
        m_copyPainter->setOpacity(origOpacity);

        const KoCompositeOp * op = painter()->compositeOp();
        painter()->setCompositeOp(COMPOSITE_COPY);

        if (m_previousRect.isNull()){
            painter()->bitBlt(rc.topLeft(),m_currentLayerDevice,rc);

            if (!rects.isEmpty()) {
                for (int i = 0 ; i < rects.size(); i++) {
                    const QRect &area = rects.at(i);
                    painter()->bitBlt(area.topLeft(),m_currentLayerDevice,area);
                    m_previousRects.append(area);
                }
            }

        } else {
            QRect unitedRc = m_previousRect.united(rc).normalized();
            painter()->bitBlt(unitedRc.topLeft(),m_currentLayerDevice,unitedRc);

            if (!rects.isEmpty()) {
                for (int i = 0 ; i < rects.size(); i++) {
                    const QRect &area = rects.at(i);
                    unitedRc = m_previousRects.at(0).united(area);
                    painter()->bitBlt(unitedRc.topLeft(),m_currentLayerDevice,unitedRc);
                    m_previousRects[i] = area;
                }
            }

        }
        m_previousRect = rc;




        painter()->setCompositeOp(op);
        transaction.revert();
    }
    m_isFirst = false;
    return kdi;
}


qreal KisExperimentPaintOp::paintAt(const KisPaintInformation& info)
{
    Q_UNUSED(info);
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
                }
                else if (curveElementCounter == 2){
                    ctrl2 = QPointF(e.x,e.y);
                    newPath.cubicTo(ctrl1,ctrl2,endPoint);
                }
                break;
            }
        }

    }// for

    return newPath;
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
