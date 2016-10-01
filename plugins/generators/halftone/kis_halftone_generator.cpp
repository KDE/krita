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
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOps.h>
#include <KoUpdater.h>
#include <kis_generator_registry.h>
#include <kis_filter_configuration.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_types.h>
#include <kis_processing_information.h>

#include "kis_halftone_generator.h"


K_PLUGIN_FACTORY_WITH_JSON(KritaHalftoneGeneratorFactory, "kritahalftonegenerator.json", registerPlugin<KritaHalftoneGenerator>();)

KritaHalftoneGenerator::KritaHalftoneGenerator(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisGeneratorRegistry::instance()->add(new KisHalftoneGenerator());
}

KritaHalftoneGenerator::~KritaHalftoneGenerator()
{
}

KisHalftoneGenerator::KisHalftoneGenerator() : KisGenerator(id(), KoID("basic"), i18n("&Halftone..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
}

KisFilterConfigurationSP KisHalftoneGenerator::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("halftonegenerator", 1);

    config->setProperty("cellSize", 8.0);
    config->setProperty("intensity", 0.5);
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

KisConfigWidget * KisHalftoneGenerator::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const
{
    return new KisHalftoneGeneratorConfigWidget(parent, dev);
}

void KisHalftoneGenerator::generate(KisProcessingInformation dstInfo,
                                    const QSize& size,
                                    const KisFilterConfigurationSP config,
                                    KoUpdater* progressUpdater) const
{
    KisPaintDeviceSP device = dstInfo.paintDevice();

    Q_ASSERT(!device.isNull());
    Q_ASSERT(config);

    QRect applyRect(QPoint(),size);

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
    if (progressUpdater) {
        progressUpdater->setRange(0, (applyRect.height()/cellSpacingV+3)*(applyRect.width()/cellSpacingH+3));
    }
    KisPainter painter(device);
    painter.setCompositeOp(device->colorSpace()->compositeOp(COMPOSITE_OVER));
    KisPaintDeviceSP dab = device->createCompositionSourceDevice();
    KisPainter dbPainter(dab);
    device->fill(applyRect, backgroundC);
    dbPainter.setAntiAliasPolygonFill(config->getBool("antiAliasing", true));
    dbPainter.setPaintColor(foregroundC);
    dbPainter.setFillStyle(KisPainter::FillStyleForegroundColor);
    dbPainter.setCompositeOp(device->colorSpace()->compositeOp(COMPOSITE_OVER));
    quint8 eightbit = 255;
    if (config->getBool("invert", false)) {
        eightbit = 0;
    }
    quint8 intensity = 255 * config->getFloat("intensity", 0.5);

    QRect cellRect(applyRect.topLeft()-QPoint(qFloor(cellSpacingH), qFloor(qMax(cellSpacingV, diameter))), applyRect.bottomRight()+QPoint(qCeil(cellSpacingH), qCeil(qMax(cellSpacingV, diameter))));
    for (int i=0; i<gridPoints.size(); i++) {
        QPointF samplePoint = gridPoints.at(i);
        if (cellRect.contains(samplePoint.toPoint())) {
            qint32 xdifference = qFloor(samplePoint.x())- applyRect.left();
            qint32 ydifference = qFloor(samplePoint.y())- applyRect.top();

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
}

//----------config---------//
KisHalftoneGeneratorConfigWidget::KisHalftoneGeneratorConfigWidget(QWidget* parent, KisPaintDeviceSP dev)
        : KisConfigWidget(parent)
{
    Q_ASSERT(dev);
    m_page.setupUi(this);

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();

    if (dev->colorSpace()) {
        cs = dev->colorSpace();
    }

    KoColor white(Qt::white, cs);
    KoColor black(Qt::black, cs);

    m_page.bnForeColor->setColor(white);
    m_page.bnBackColor->setColor(black);
    m_page.bnForeColor->setDefaultColor(white);
    m_page.bnBackColor->setDefaultColor(black);

    m_page.sldSize->setRange(3, 90);
    m_page.sldIntensity->setRange(0, 100);
    m_page.sldIntensity->setSingleStep(1);
    m_page.sldIntensity->setColors(white, black);

    connect(m_page.sldSize, SIGNAL(valueChanged(int)), SLOT(slotConfigChanged()));
    connect(m_page.dialAngle, SIGNAL(valueChanged(int)), m_page.spAngle, SLOT(setValue(int)));
    connect(m_page.dialAngle, SIGNAL(valueChanged(int)), SLOT(slotConfigChanged()));
    connect(m_page.spAngle, SIGNAL(valueChanged(int)), SLOT(slotConfigChanged()));

    //connect(m_page.bnForeColor, SIGNAL(changed(KoColor)), SLOT(slotUpdateSlider()));
    //connect(m_page.bnBackColor, SIGNAL(changed(KoColor)), SLOT(slotUpdateSlider()));

    connect(m_page.bnForeColor, SIGNAL(changed(KoColor)), SLOT(slotConfigChanged()));
    connect(m_page.bnBackColor, SIGNAL(changed(KoColor)), SLOT(slotConfigChanged()));
    connect(m_page.chkAntiAliasing, SIGNAL(toggled(bool)), SLOT(slotConfigChanged()));
    connect(m_page.chkInvert, SIGNAL(toggled(bool)), SLOT(slotConfigChanged()));
}

KisHalftoneGeneratorConfigWidget::~KisHalftoneGeneratorConfigWidget()
{

}

KisPropertiesConfigurationSP KisHalftoneGeneratorConfigWidget::configuration() const
{
    KisFilterConfiguration *config = new KisFilterConfiguration("halftonegenerator", 1);
    config->setProperty("cellSize", m_page.sldSize->value());
    config->setProperty("patternAngle", m_page.spAngle->value());
    config->setProperty("intensity", (float)m_page.sldIntensity->value()/100.0);
    QVariant v;
    v.setValue(m_page.bnForeColor->color());
    config->setProperty("foreGroundColor", v);
    v.setValue(m_page.bnBackColor->color());
    config->setProperty("backGroundColor", v);
    config->setProperty("antiAliasing", m_page.chkAntiAliasing->isChecked());
    config->setProperty("invert", m_page.chkInvert->isChecked());

    return config;
}

void KisHalftoneGeneratorConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    if (config->getProperty("cellSize", value)) {
        m_page.sldSize->setValue(value.toUInt());
    }
    if (config->getProperty("patternAngle", value)) {
        m_page.dialAngle->setValue(value.toUInt());
        m_page.spAngle->setValue(value.toUInt());
    }
    if (config->getProperty("antiAliasing", value)) {
        m_page.chkAntiAliasing->setChecked(value.toBool());
    }
    if (config->getProperty("invert", value)) {
        m_page.chkInvert->setChecked(value.toBool());
    }
    if (config->getProperty("intensity", value)) {
        m_page.sldIntensity->setValue(value.toFloat()*100);
    }


    m_page.bnForeColor->setColor(config->getColor("foreGroundColor",m_page.bnForeColor->defaultColor()));
    m_page.bnBackColor->setColor(config->getColor("backGroundColor",m_page.bnBackColor->defaultColor()));
}


#include "kis_halftone_generator.moc"
