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

#include <qrect.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qtabwidget.h>
#include <qtoolbutton.h>
#include <qslider.h>

#include <kdebug.h>

#include "kis_colorspace_factory_registry.h"
#include "kcurve.h"
#include "kis_brush.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_layer.h"
#include "kis_meta_registry.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_paintop.h"
#include "kis_input_device.h"
#include "kis_selection.h"

#include "kis_dlgbrushcurvecontrol.h"

KisPaintOp * KisSmudgeOpFactory::createOp(const KisPaintOpSettings *settings, KisPainter * painter)
{
    const KisSmudgeOpSettings *brushopSettings = dynamic_cast<const KisSmudgeOpSettings *>(settings);
    Q_ASSERT(settings == 0 || brushopSettings != 0);

    KisPaintOp * op = new KisSmudgeOp(brushopSettings, painter);
    Q_CHECK_PTR(op);
    return op;
}

KisSmudgeOpSettings::KisSmudgeOpSettings(QWidget *parent, bool isTablet)
    : super(parent)
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
        m_curveControl = new WdgBrushCurveControl(m_optionsWidget);
        // We abuse the darken curve here for rate
        m_curveControl->tabWidget->setTabLabel(m_curveControl->tabWidget->page(2), i18n("Rate"));
        m_curveControl->tabWidget->setTabToolTip(m_curveControl->tabWidget->page(2),
                i18n("Modifies the rate. Bottom is 0% of the rate top is 100% of the original rate."));
        QToolButton* moreButton = new QToolButton(Qt::UpArrow, m_optionsWidget);
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
    if (m_curveControl->exec() == QDialog::Accepted) {
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

KisPaintOpSettings* KisSmudgeOpFactory::settings(QWidget * parent, const KisInputDevice& inputDevice)
{
    if (inputDevice == KisInputDevice::mouse()) {
        // No options for mouse, only tablet devices
        return new KisSmudgeOpSettings(parent, false);
    } else {
        return new KisSmudgeOpSettings(parent, true);
    }
}

KisSmudgeOp::KisSmudgeOp(const KisSmudgeOpSettings *settings, KisPainter *painter)
    : super(painter)
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
    KisPaintDeviceSP device = m_painter->device();
        m_srcdev = new KisPaintDevice(device->colorSpace(), "duplicate source dev");
        m_target = new KisPaintDevice(device->colorSpace(), "duplicate target dev");
}

KisSmudgeOp::~KisSmudgeOp()
{
}

void KisSmudgeOp::paintAt(const KisPoint &pos, const KisPaintInformation& info)
{
    KisPaintInformation adjustedInfo(info);
    if (!m_pressureSize)
        adjustedInfo.pressure = PRESSURE_DEFAULT;
    else if (m_customSize)
        adjustedInfo.pressure = scaleToCurve(adjustedInfo.pressure, m_sizeCurve);

    // Painting should be implemented according to the following algorithm:
    // retrieve brush
    // if brush == mask
    //          retrieve mask
    // else if brush == image
    //          retrieve image
    // subsample (mask | image) for position -- pos should be double!
    // apply filters to mask (colour | gradient | pattern | etc.
    // composite filtered mask into temporary layer
    // composite temporary layer into target layer
    // @see: doc/brush.txt

    if (!m_painter->device()) return;

    KisBrush *brush = m_painter->brush();
    
    Q_ASSERT(brush);
    if (!brush) return;
    if (! brush->canPaintFor(adjustedInfo) )
        return;
    
    KisPaintDeviceSP device = m_painter->device();

    KisPoint hotSpot = brush->hotSpot(adjustedInfo);
    KisPoint pt = pos - hotSpot;

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    Q_INT32 x;
    double xFraction;
    Q_INT32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    KisPaintDeviceSP dab = 0;

    Q_UINT8 origOpacity = m_painter->opacity();

    if (m_pressureOpacity) {
        if (!m_customOpacity)
            m_painter->setOpacity((Q_INT8)(origOpacity * info.pressure));
        else
            m_painter->setOpacity((Q_INT8)(origOpacity * scaleToCurve(info.pressure, m_opacityCurve)));
    }

    if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        dab = brush->image(device->colorSpace(), adjustedInfo, xFraction, yFraction);
        dab->convertTo(KisMetaRegistry::instance()->csRegistry()->getAlpha8());
    }
    else {
        KisAlphaMaskSP mask = brush->mask(adjustedInfo, xFraction, yFraction);
        dab = computeDab(mask, KisMetaRegistry::instance()->csRegistry()->getAlpha8());
    }

    
    m_painter->setPressure(adjustedInfo.pressure);

    QRect dabRect = QRect(0, 0, brush->maskWidth(adjustedInfo),
                          brush->maskHeight(adjustedInfo));
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());

    KisImage * image = device->image();
    
    if (image != 0) {
        dstRect &= image->bounds();
    }
    
    Q_INT32 sw = dab->extent().width();
    Q_INT32 sh = dab->extent().height();

    KisPainter copyPainter(m_srcdev);
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
        opacity = OPACITY_OPAQUE - opacity;
    } else {
        m_firstRun = false;
    }
    copyPainter.bitBlt(0, 0, COMPOSITE_OVER, device, opacity, pt.x(), pt.y(), sw, sh);
    copyPainter.end();
    
        m_target = new KisPaintDevice(device->colorSpace(), "duplicate target dev");
    
    copyPainter.begin(m_target);
    

    copyPainter.bltMask(0, 0, COMPOSITE_OVER, m_srcdev, dab,
                             OPACITY_OPAQUE, 0, 0, sw, sh);
    copyPainter.end();

    
    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    Q_INT32 sx = dstRect.x() - x;
    Q_INT32 sy = dstRect.y() - y;
    sw = dstRect.width();
    sh = dstRect.height();

    if (m_source->hasSelection()) {
        m_painter->bltSelection(dstRect.x(), dstRect.y(), m_painter->compositeOp(), m_target,
                                m_source->selection(), m_painter->opacity(), sx, sy, sw, sh);
    }
    else {
        m_painter->bitBlt(dstRect.x(), dstRect.y(), m_painter->compositeOp(), m_target, m_painter->opacity(), sx, sy, sw, sh);
    }

    m_painter->addDirtyRect(dstRect);

    m_painter->setOpacity(origOpacity);
    
}

#include "kis_smudgeop.moc"
