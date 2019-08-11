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
#include <KoCompositeOps.h>

#include <filter/kis_filter_category_ids.h>
#include "kis_filter_configuration.h"
#include <kis_filter_registry.h>

#include <kis_random_accessor_ng.h>
#include <kis_sequential_iterator.h>
#include <kis_types.h>
#include <kis_painter.h>
#include <kis_pixel_selection.h>
#include <kis_selection.h>

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
    : KisFilter(id(), FiltersCategoryArtisticId, i18n("&Halftone..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);

    setSupportsPainting(false);
    setShowConfigurationWidget(true);
    setSupportsLevelOfDetail(false);
    setSupportsAdjustmentLayers(false);
    setSupportsThreading(false);
}

//I am pretty terrible at trigonometry, hence all the comments.
void KisHalftoneFilter::processImpl(KisPaintDeviceSP device,
                                    const QRect &applyRect,
                                    const KisFilterConfigurationSP config,
                                    KoUpdater *progressUpdater) const
{
    qreal cellSize = (qreal)config->getInt("cellSize", 8);
    qreal angle = fmod((qreal)config->getInt("patternAngle", 45), 90.0);
    KoColor foregroundC(Qt::black, device->colorSpace());
    foregroundC.fromKoColor(config->getColor("foreGroundColor", KoColor(Qt::black, device->colorSpace()) ) );
    KoColor backgroundC(Qt::white, device->colorSpace());
    backgroundC.fromKoColor(config->getColor("backGroundColor", KoColor(Qt::white, device->colorSpace()) ) );

    //First calculate the full diameter, using pythagoras theorem.
    qreal diameter = qSqrt((cellSize*cellSize)*2);

    qreal cellSpacingH = cellSize;
    qreal cellSpacingV = cellSize;
    qreal cellOffsetV = 0;

    if (angle>0.0){
        //2/3=0.6 sin=o/h
        //0.6*3=2 sin*h=o
        //2/0.6=3 o/sin=h
        qreal angleRad = qDegreesToRadians(angle);
        //Horizontal cellspacing is the hypotenuse of the Angle and the cellSize(opposite).
        cellSpacingH = cellSize/qSin(angleRad);
        //Vertical cellspacing is the opposite of the Angle and the cellSize(hypotenuse).
        cellSpacingV = cellSize*qSin(angleRad);
        //cellSpacingoffset is the oppose of the (90-angle) and the vertical cellspacing(adjectant) toa t=o/a, t*a=o.
        cellOffsetV = qTan(qDegreesToRadians(90-angle))*cellSpacingV;
    }

    QPolygonF gridPoints;
    QRect totalRect(QPoint(0,0), applyRect.bottomRight());
    if (gridPoints.size()<1) {
        int rows = (totalRect.height()/cellSpacingV)+3;
        for (int r=0; r<rows; r++) {
            qreal offset = fmod(((qreal)r*cellOffsetV), cellSpacingH);
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
    painter.setCompositeOp(device->colorSpace()->compositeOp(COMPOSITE_OVER));
    KisPaintDeviceSP dab = device->createCompositionSourceDevice();
    KisPainter dbPainter(dab);
    KisSelectionSP alpha = new KisSelection(new KisSelectionEmptyBounds(0));
    alpha->pixelSelection()->copyAlphaFrom(device, applyRect);
    device->fill(applyRect, backgroundC);
    dbPainter.setAntiAliasPolygonFill(config->getBool("antiAliasing", true));
    dbPainter.setPaintColor(foregroundC);
    dbPainter.setFillStyle(KisPainter::FillStyleForegroundColor);
    dbPainter.setCompositeOp(device->colorSpace()->compositeOp(COMPOSITE_OVER));
    quint8 eightbit = 255;
    if (config->getBool("invert", false)) {
        eightbit = 0;
    }

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
            qreal size = diameter*((qAbs(intensity-eightbit))/255.0);
            QPoint sPoint(qMax(qFloor(samplePoint.x()), applyRect.left()),
                          qMax(qFloor(samplePoint.y()), applyRect.top()));
            dbPainter.bitBlt(0, 0, device,
                             sPoint.x(),
                             sPoint.y(),
                             diameter,
                             diameter);
            dbPainter.paintEllipse((samplePoint.x()-qFloor(samplePoint.x()))+qCeil(size)-size,
                                   (samplePoint.y()-qFloor(samplePoint.y()))+qCeil(size)-size,
                                   size,
                                   size);
            dab->crop(qAbs(qMin(0, xdifference)),
                      qAbs(qMin(0, ydifference)),
                      diameter,
                      diameter);
            //we only want to paint the bits actually in the apply rect...)
            painter.bitBlt(sPoint,
                           dab,
                           dab->exactBounds());
            if (progressUpdater) {
                progressUpdater->setValue(i);
            }
        }
    }
    alpha->pixelSelection()->invert();
    device->clearSelection(alpha);
}

KisFilterConfigurationSP KisHalftoneFilter::factoryConfiguration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("halftone", 1);
    config->setProperty("cellSize", 8.0);
    config->setProperty("patternAngle", 45.0);
    QVariant v;
    KoColor black;
    black.fromQColor(QColor(Qt::black));
    v.setValue(black);
    config->setProperty("foreGroundColor", v);
    KoColor white;
    white.fromQColor(QColor(Qt::white));
    v.setValue(white);
    config->setProperty("backGroundColor", v);
    config->setProperty("antiAliasing", true);
    config->setProperty("invert", false);

    return config;
}

KisConfigWidget *KisHalftoneFilter::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool) const
{
    return new KisHalftoneConfigWidget(parent, dev);
}

//----------config---------//
KisHalftoneConfigWidget::KisHalftoneConfigWidget(QWidget *parent, KisPaintDeviceSP dev)
    : KisConfigWidget(parent)
{
    Q_ASSERT(dev);
    m_page.setupUi(this);

    KoColor white(Qt::white, dev->colorSpace());
    KoColor black(Qt::black, dev->colorSpace());

    m_page.bnforeground->setColor(white);
    m_page.bnbackground->setColor(black);
    m_page.bnforeground->setDefaultColor(white);
    m_page.bnbackground->setDefaultColor(black);

    m_page.sld_cellSize->setRange(3, 90);

    connect(m_page.sld_cellSize, SIGNAL(valueChanged(int)), SLOT(slotConfigChanged()));
    connect(m_page.dial_angle, SIGNAL(valueChanged(int)), m_page.spb_angle, SLOT(setValue(int)));
    connect(m_page.dial_angle, SIGNAL(valueChanged(int)), SLOT(slotConfigChanged()));
    connect(m_page.spb_angle, SIGNAL(valueChanged(int)), SLOT(slotConfigChanged()));
    connect(m_page.bnforeground, SIGNAL(changed(KoColor)), SLOT(slotConfigChanged()));
    connect(m_page.bnbackground, SIGNAL(changed(KoColor)), SLOT(slotConfigChanged()));
    connect(m_page.ckbAntialiasing, SIGNAL(toggled(bool)), SLOT(slotConfigChanged()));
    connect(m_page.ckbInvert, SIGNAL(toggled(bool)), SLOT(slotConfigChanged()));
}

KisHalftoneConfigWidget::~KisHalftoneConfigWidget()
{

}

KisPropertiesConfigurationSP KisHalftoneConfigWidget::configuration() const
{
    KisFilterConfiguration *config = new KisFilterConfiguration("halftone", 1);
    config->setProperty("cellSize", m_page.sld_cellSize->value());
    config->setProperty("patternAngle", m_page.spb_angle->value());
    QVariant v;
    v.setValue(m_page.bnforeground->color());
    config->setProperty("foreGroundColor", v);
    v.setValue(m_page.bnbackground->color());
    config->setProperty("backGroundColor", v);
    config->setProperty("antiAliasing", m_page.ckbAntialiasing->isChecked());
    config->setProperty("invert", m_page.ckbInvert->isChecked());

    return config;
}

void KisHalftoneConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    if (config->getProperty("cellSize", value)) {
        m_page.sld_cellSize->setValue(value.toUInt());
    }
    if (config->getProperty("patternAngle", value)) {
        m_page.dial_angle->setValue(value.toUInt());
        m_page.spb_angle->setValue(value.toUInt());
    }
    if (config->getProperty("antiAliasing", value)) {
        m_page.ckbAntialiasing->setChecked(value.toBool());
    }
    if (config->getProperty("invert", value)) {
        m_page.ckbInvert->setChecked(value.toBool());
    }


    m_page.bnforeground->setColor(config->getColor("foreGroundColor",m_page.bnforeground->defaultColor()));
    m_page.bnbackground->setColor(config->getColor("backGroundColor",m_page.bnbackground->defaultColor()));
}

#include "kis_halftone_filter.moc"
