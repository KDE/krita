/*
 * This file is part of Krita
 *
 * Copyright (c) 2016 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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
#include <cmath>
#include <QtMath>

#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <KoUpdater.h>

#include "kis_filter_configuration.h"
#include <kis_filter_registry.h>

#include <kis_random_accessor_ng.h>
#include <kis_sequential_iterator.h>
#include <kis_types.h>
#include <kis_painter.h>

#include "kis_halftone_filter.h"

K_PLUGIN_FACTORY_WITH_JSON(KritaHalftoneFactory, "kritahalftone.json", registerPlugin<KritaHalftone>();)

KritaHalftone::KritaHalftone(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisHalftoneFilter());
}

KritaHalftone::~KritaHalftone()
{
}

KisHalftoneFilter::KisHalftoneFilter()
    : KisFilter(id(), categoryArtistic(), i18n("&Halftone..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);

    setSupportsPainting(false);
    setShowConfigurationWidget(false);
    setSupportsLevelOfDetail(false);
    setSupportsAdjustmentLayers(true);
    setSupportsThreading(false);
}

//I am pretty terrible at trigionometry, hence all the comments.
void KisHalftoneFilter::processImpl(KisPaintDeviceSP device,
                                    const QRect &applyRect,
                                    const KisFilterConfiguration *config,
                                    KoUpdater *progressUpdater) const
{
    qreal cellSize = config->getFloat("cellSize", 8.0);
    qreal angle = config->getFloat("patternAngle", 30.0);

    //First calculate the full diameter, using pythagoras theorem.
    qreal diameter = qSqrt((cellSize*cellSize)*2);

    //Horizontal cellspacing is the hypotenuse of the Angle and the cellSize.
    qreal cellSpacingH = cellSize/qSin(angle);

    //Vertical cellspacing is the opposite of the Angle and the cellSize.
    qreal cellSpacingV = qSin(angle)*cellSize;
    //cellSpacingoffset is the adjectant of the angle and the vertical cellspacing.
    qreal cellOffsetV = qTan(angle)*cellSpacingV;
    QPolygonF gridPoints;
    QRect totalRect(QPoint(0,0), applyRect.bottomRight());
    if (gridPoints.size()<1) {
        int rows = (totalRect.height()/cellSpacingV)+3;
        for (int r=0; r<rows; r++) {
            qreal offset = fmod(((qreal)r*cellOffsetV), cellSize);
            int columns = ((totalRect.width()+offset)/cellSpacingH)+3;
            for (int c = 0; c<columns; c++) {
                gridPoints.append(QPointF((c*cellSpacingH)+offset-cellSpacingH,
                                            r*cellSpacingV-cellSpacingV));
            }
        }
    }
    //qDebug()<<applyRect;
    //qDebug()<<device->exactBounds();
    if (progressUpdater) {
        progressUpdater->setRange(0, (applyRect.height()/cellSpacingV+3)*(applyRect.width()/cellSpacingH+3));
    }
    KisRandomConstAccessorSP itterator = device->createRandomConstAccessorNG( 0, 0);
    //itterator->numContiguousColumns(qCeil(cellSize));
    //itterator->numContiguousRows(qCeil(cellSize));
    KisPainter painter(device);
    KisPaintDeviceSP dab = device->createCompositionSourceDevice();
    KisPainter dbPainter(dab);
    KoColor white(Qt::white, device->colorSpace());
    KoColor black(Qt::black, device->colorSpace());
    device->fill(applyRect, white);
    dbPainter.setPaintColor(black);
    dbPainter.setFillStyle(KisPainter::FillStyleForegroundColor);

    QRect cellRect(applyRect.topLeft()-QPoint(qFloor(cellSpacingH), qFloor(qMax(cellSpacingV, diameter))), applyRect.bottomRight()+QPoint(qCeil(cellSpacingH), qCeil(qMax(cellSpacingV, diameter))));
    for (int i=0; i<gridPoints.size(); i++) {
        QPointF samplePoint = gridPoints.at(i);
        if (cellRect.contains(samplePoint.toPoint())) {
            qint32 xdifference = qFloor(samplePoint.x())- applyRect.left();
            qint32 ydifference = qFloor(samplePoint.y())- applyRect.top();
            QPoint center(qBound(applyRect.left()+1, qFloor(samplePoint.x())+qCeil(cellSize*0.5), applyRect.right()-1),
                          qBound(applyRect.top()+1, qFloor(samplePoint.y())+qCeil(cellSize*0.5), applyRect.bottom()-1));
            itterator->moveTo(center.x(), center.y());
            quint8 intensity = device->colorSpace()->intensity8(itterator->oldRawData());
            qreal size = diameter*((255-(intensity))/255.0);
            QPoint sPoint(qMax(qFloor(samplePoint.x()), applyRect.left()),
                          qMax(qFloor(samplePoint.y()), applyRect.top()));
            dbPainter.bitBlt(0, 0, device,
                             sPoint.x(),
                             sPoint.y(),
                             diameter,
                             diameter);
            dbPainter.paintEllipse(0, 0, qCeil(size), qCeil(size));
            dab->crop(qAbs(qMin(0, xdifference)),
                      qAbs(qMin(0, ydifference)),
                      diameter,
                      diameter);
            //we only want to paint the bits actually in the apply rect...)
            painter.bitBlt(sPoint,
                           dab,
                           dab->exactBounds());
            //painter.paintEllipse(samplePoint.x(), samplePoint.y(), size, size);
            if (progressUpdater) {
                progressUpdater->setValue(i);
            }
        }
    }
}

KisFilterConfiguration *KisHalftoneFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration *config = new KisFilterConfiguration("halftone", 1);
    config->setProperty("cellSize", 8.0);
    config->setProperty("patternAngle", 45.0);
    return config;
}

/*KisConfigWidget *KisHalftoneFilter::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev) const
{
}*/

#include "kis_halftone_filter.moc"
