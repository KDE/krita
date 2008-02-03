/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_smudgeop.h"

#include <string.h>

#include <QSlider>
#include <QRect>
#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include <QDomElement>
#include <QHBoxLayout>
#include <QToolButton>

#include <kdebug.h>

#include "KoColorSpaceRegistry.h"
#include "kcurve.h"
#include "kis_brush.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_layer.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_paintop.h"
#include "KoInputDevice.h"
#include "kis_selection.h"
#include <kis_iterator.h>
#include <kis_iterators_pixel.h>
#include "ui_kis_dlgbrushcurvecontrol.h"
#include <kis_properties_configuration.h>

KisPaintOp * KisSmudgeOpFactory::createOp(const KisPaintOpSettings *settings, KisPainter * painter, KisImageSP image)
{
    const KisSmudgeOpSettings *brushopSettings = dynamic_cast<const KisSmudgeOpSettings *>(settings);
    Q_ASSERT(settings == 0 || brushopSettings != 0);

    KisPaintOp * op = new KisSmudgeOp(brushopSettings, painter);
    Q_CHECK_PTR(op);
    return op;
}

KisSmudgeOpSettings::KisSmudgeOpSettings(QWidget *parent, bool isTablet)
    : KisPaintOpSettings()
{
    m_optionsWidget = new QWidget(parent, "brush option widget");
    QHBoxLayout * l = new QHBoxLayout(m_optionsWidget);
    l->setAutoAdd(true);
    m_rateLabel = new QLabel(i18n("Rate: "), m_optionsWidget);
    m_rateSlider = new QSlider(0,100,1, 50, Qt::Horizontal, m_optionsWidget);
    if(isTablet)
    {
        m_pressureVariation = new QLabel(i18n("Pressure variation: "), m_optionsWidget);
        m_size =  new QCheckBox(i18n("Size"), m_optionsWidget);
        m_size->setChecked(true);
        m_opacity = new QCheckBox(i18n("Opacity"), m_optionsWidget);
        m_rate =  new QCheckBox(i18n("Rate"), m_optionsWidget);
        m_curveControl = new Ui::WdgBrushCurveControl();
        m_curveControlWidget = new QDialog(m_optionsWidget);
        m_curveControl->setupUi(m_curveControlWidget);
        // We abuse the darken curve here for rate
        m_curveControl->tabWidget->setTabLabel(m_curveControl->tabWidget->page(2), i18n("Rate"));
        m_curveControl->tabWidget->setTabToolTip(m_curveControl->tabWidget->page(2),
                i18n("Modifies the rate. Bottom is 0% of the rate top is 100% of the original rate."));
        QToolButton* moreButton = new QToolButton(m_optionsWidget);
        moreButton->setArrowType(Qt::UpArrow);
        moreButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        moreButton->setMinimumSize(QSize(24,24)); // Bah, I had hoped the above line would make this unneeded
    
        connect(moreButton, SIGNAL(clicked()), this, SLOT(slotCustomCurves()));
    } else {
        m_pressureVariation = 0;
        m_size = 0;
        m_rate = 0;
        m_opacity = 0;
        m_curveControl = 0;
    }

    m_customRate = false;
    m_customSize = false;
    m_customOpacity = false;
    // the curves will get filled in when the slot gets accepted
}

void KisSmudgeOpSettings::slotCustomCurves() {
    if (m_curveControlWidget->exec() == QDialog::Accepted) {
        m_customRate = m_curveControl->darkenCheckbox->isChecked();
        m_customSize = m_curveControl->sizeCheckbox->isChecked();
        m_customOpacity = m_curveControl->opacityCheckbox->isChecked();

        if (m_customRate) {
            transferCurve(m_curveControl->darkenCurve, m_rateCurve);
        }
        if (m_customSize) {
            transferCurve(m_curveControl->sizeCurve, m_sizeCurve);
        }
        if (m_customOpacity) {
            transferCurve(m_curveControl->opacityCurve, m_opacityCurve);
        }
    }
}

void KisSmudgeOpSettings::transferCurve(KCurve* curve, double* target) {
    double value;
    for (int i = 0; i < 256; i++) {
        value = curve->getCurveValue( i / 255.0);
        if (value < PRESSURE_MIN)
            target[i] = PRESSURE_MIN;
        else if (value > PRESSURE_MAX)
            target[i] = PRESSURE_MAX;
        else
            target[i] = value;
    }
}

int KisSmudgeOpSettings::rate() const
{
    return m_rateSlider->value();
}

bool KisSmudgeOpSettings::varyRate() const
{
    return m_rate ? m_rate->isChecked() : false;
}

bool KisSmudgeOpSettings::varySize() const
{
    return m_size ? m_size->isChecked() : true;
}

bool KisSmudgeOpSettings::varyOpacity() const
{
    return m_opacity ? m_opacity->isChecked() : false;
}

void KisSmudgeOpSettings::fromXML(const QDomElement& elt)
{
    QDomElement e = elt.firstChildElement("Params");
    if(not e.isNull())
    {
        KisPropertiesConfiguration kpc;
        kpc.fromXML(e);
        m_size->setChecked( kpc.getBool( "PressureSize", false) );
        m_opacity->setChecked( kpc.getBool( "PressureOpacity", false) );
        m_customSize = kpc.getBool( "CustomSize", false);
        m_customOpacity = kpc.getBool( "CustomOpacity", false);
        for(int i = 0; i < 256; i++)
        {
            if( m_customSize )
                m_sizeCurve[i] = kpc.getDouble( QString("SizeCurve%0").arg(i), i / 255.0 );
            if( m_customOpacity )
                m_opacityCurve[i] = kpc.getDouble( QString("OpacityCurve%0").arg(i), i / 255.0 );
        }
    }
}

void KisSmudgeOpSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    KisPropertiesConfiguration kpc;
    kpc.setProperty("PressureSize", m_size->isChecked());
    kpc.setProperty("PressureOpacity", m_opacity->isChecked());
    kpc.setProperty("CustomSize", m_customSize);
    kpc.setProperty("CustomOpacity", m_customOpacity);
    
    for(int i = 0; i < 256; i++)
    {
        if( m_customSize )
            kpc.setProperty( QString("SizeCurve%0").arg(i), m_sizeCurve[i] );
        if( m_customOpacity )
            kpc.setProperty( QString("OpacityCurve%0").arg(i), m_opacityCurve[i] );
    }
    
    QDomElement paramsElt = doc.createElement( "Params" );
    rootElt.appendChild( paramsElt );
    kpc.toXML( doc, paramsElt);
}


KisPaintOpSettings* KisSmudgeOpFactory::settings(QWidget * parent, const KoInputDevice& inputDevice, KisImageSP /*image*/)
{
    if (inputDevice == KoInputDevice::mouse()) {
        // No options for mouse, only tablet devices
        return new KisSmudgeOpSettings(parent, false);
    } else {
        return new KisSmudgeOpSettings(parent, true);
    }
}

KisSmudgeOp::KisSmudgeOp(const KisSmudgeOpSettings *settings, KisPainter *painter)
    : KisPaintOp(painter)
    , m_firstRun(true)
    , m_rate(50)
    , m_pressureSize(true)
    , m_pressureRate(false)
    , m_pressureOpacity(false)
    , m_customRate(false)
    , m_customSize(false)
    , m_customOpacity(false)
    , m_target(0)
    , m_srcdev(0)
{
    if (settings != 0) {
        m_rate = settings->rate();
        m_pressureRate = settings->varyRate();
        m_pressureSize = settings->varySize();
        m_pressureOpacity = settings->varyOpacity();
        m_customRate = settings->customRate();
        m_customSize = settings->customSize();
        m_customOpacity = settings->customOpacity();
        if (m_customSize) {
            memcpy(m_sizeCurve, settings->sizeCurve(), 256 * sizeof(double));
        }
        if (m_customOpacity) {
            memcpy(m_opacityCurve, settings->opacityCurve(), 256 * sizeof(double));
        }
        if (m_customRate) {
            memcpy(m_rateCurve, settings->rateCurve(), 256 * sizeof(double));
        }
    }
    KisPaintDeviceSP device = painter->device();
    m_srcdev = new KisPaintDevice(device->colorSpace(), "duplicate source dev");
    m_target = new KisPaintDevice(device->colorSpace(), "duplicate target dev");
}

KisSmudgeOp::~KisSmudgeOp()
{
}

void KisSmudgeOp::paintAt(const KisPaintInformation& info)
{
    KisPaintInformation adjustedInfo(info);
    if (!m_pressureSize)
        adjustedInfo.setPressure( PRESSURE_DEFAULT );

    if (!painter()->device()) return;

    KisBrush *brush = painter()->brush();

    Q_ASSERT(brush);
    if (!brush) return;
    if (! brush->canPaintFor(adjustedInfo) )
        return;

    KisPaintDeviceSP device = painter()->device();

    double pScale = KisPaintOp::scaleForPressure( adjustedInfo.pressure() );
    QPointF hotSpot = brush->hotSpot( pScale, pScale );
    QPointF pt = info.pos() - hotSpot;

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    qint32 x;
    double xFraction;
    qint32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    KisPaintDeviceSP dab = KisPaintDeviceSP(0);

    quint8 origOpacity = painter()->opacity();

    if (m_pressureOpacity) {
        if (!m_customOpacity)
            painter()->setOpacity((Q_INT8)(origOpacity * info.pressure()));
        else
            painter()->setOpacity((Q_INT8)(origOpacity * scaleToCurve(info.pressure(), m_opacityCurve)));
    }

    if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        dab = brush->image(device->colorSpace(), pScale, 0.0, adjustedInfo, xFraction, yFraction);
        dab->convertTo(KoColorSpaceRegistry::instance()->alpha8());
    }
    else {
        KisAlphaMaskSP mask = brush->mask(adjustedInfo, xFraction, yFraction);
        dab = cachedDab( );
        KoColor color = painter()->paintColor();
        color.convertTo( dab->convertTo(KoColorSpaceRegistry::instance()->alpha8()) );
        brush->mask(dab, color, scale, pScale, 0.0, info, xFraction, yFraction);
    }

    
    painter()->setPressure(adjustedInfo.pressure);

    QRect dabRect = QRect(0, 0, brush->maskWidth(adjustedInfo),
                          brush->maskHeight(adjustedInfo));
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());

    KisImage * image = device->image();
    
    if (image != 0) {
        dstRect &= image->bounds();
    }
    
    Q_INT32 sw = dab->extent().width();
    Q_INT32 sh = dab->extent().height();

    int opacity = OPACITY_OPAQUE;
    if(!m_firstRun)
    {
        opacity = rate();
        if (m_pressureRate) {
            if (m_customRate) {
                opacity = CLAMP((Q_UINT8)(double(opacity) * scaleToCurve(info.pressure, m_rateCurve)), OPACITY_TRANSPARENT, OPACITY_OPAQUE);
            } else {
                opacity = CLAMP((Q_UINT8)(double(opacity) * info.pressure), OPACITY_TRANSPARENT, OPACITY_OPAQUE);
            }
        }
        KisRectIterator it = m_srcdev->createRectIterator(0, 0, sw, sh, true);
        KoColorSpace* cs = m_srcdev->colorSpace();
        while(not it.isDone())
        {
//kdDebug() << ((cs->getAlpha(it.rawData()) * opacity) / OPACITY_OPAQUE) << endl;
            cs->setAlpha(it.rawData(), (cs->getAlpha(it.rawData()) * opacity) / OPACITY_OPAQUE, 1);
            ++it;
        }
        opacity = OPACITY_OPAQUE - opacity;

    } else {
        m_firstRun = false;
    }

    KisPainter copyPainter(m_srcdev);
    copyPainter.bitBlt(0, 0, COMPOSITE_OVER, device, opacity, pt.x(), pt.y(), sw, sh);
    copyPainter.end();
    
    m_target = new KisPaintDevice(device->colorSpace(), "duplicate target dev");
    
    copyPainter.begin(m_target);

    copyPainter.bltSelection(0, 0, COMPOSITE_OVER, m_srcdev, dab,
                             OPACITY_OPAQUE, 0, 0, sw, sh);
    copyPainter.end();

    
    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    Q_INT32 sx = dstRect.x() - x;
    Q_INT32 sy = dstRect.y() - y;
    sw = dstRect.width();
    sh = dstRect.height();

    if (m_source->hasSelection()) {
        painter()->bltSelection(dstRect.x(), dstRect.y(), painter()->compositeOp(), m_target,
                                m_source->selection(), painter()->opacity(), sx, sy, sw, sh);
    }
    else {
        painter()->bitBlt(dstRect.x(), dstRect.y(), painter()->compositeOp(), m_target, painter()->opacity(), sx, sy, sw, sh);
    }

    painter()->addDirtyRect(dstRect);

    painter()->setOpacity(origOpacity);
    
}

#include "kis_smudgeop.moc"
