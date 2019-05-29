/*
 *  Copyright (c) 2004,2007,2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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


#include <compositeops/KoVcMultiArchBuildSupport.h> //MSVC requires that Vc come first
#include "kis_auto_brush_widget.h"

#include <kconfig.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include <math.h>
#include <kis_debug.h>
#include <QSpinBox>
#include <QToolButton>
#include <QImage>
#include <QComboBox>
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

#include <kis_fixed_paint_device.h>
#include <kis_mask_generator.h>
#include <kis_slider_spin_box.h>
#include "kis_signals_blocker.h"
#include "kis_signal_compressor.h"
#include "kis_aspect_ratio_locker.h"


#define showSlider(input, step) input->setRange(input->minimum(), input->maximum(), step)
#include <kis_cubic_curve.h>

KisAutoBrushWidget::KisAutoBrushWidget(QWidget *parent, const char* name)
    : KisWdgAutoBrush(parent, name)
    , m_autoBrush(0)
    , m_updateCompressor(new KisSignalCompressor(100, KisSignalCompressor::FIRST_ACTIVE))
    , m_fadeAspectLocker(new KisAspectRatioLocker())
{
    connect(m_updateCompressor.data(), SIGNAL(timeout()), SLOT(paramChanged()));

    connect((QObject*)comboBoxShape, SIGNAL(activated(int)), m_updateCompressor.data(), SLOT(start()));

    inputRadius->setRange(0.01, KSharedConfig::openConfig()->group("").readEntry("maximumBrushSize", 1000), 2);
    inputRadius->setExponentRatio(3.0);
    inputRadius->setSingleStep(1);
    inputRadius->setValue(5);
    inputRadius->setSuffix(i18n(" px"));
    inputRadius->setBlockUpdateSignalOnDrag(true);
    connect(inputRadius, SIGNAL(valueChanged(qreal)), m_updateCompressor.data(), SLOT(start()));

    inputRatio->setRange(0.0, 1.0, 2);
    inputRatio->setSingleStep(0.1);
    inputRatio->setValue(1.0);
    inputRatio->setBlockUpdateSignalOnDrag(true);
    connect(inputRatio, SIGNAL(valueChanged(qreal)), m_updateCompressor.data(), SLOT(start()));

    inputHFade->setRange(0.0, 1.0, 2);
    inputHFade->setSingleStep(0.1);
    inputHFade->setValue(0.5);

    inputVFade->setRange(0.0, 1.0, 2);
    inputVFade->setSingleStep(0.1);
    inputVFade->setValue(0.5);

    aspectButton->setKeepAspectRatio(true);

    m_fadeAspectLocker->connectSpinBoxes(inputHFade, inputVFade, aspectButton);
    m_fadeAspectLocker->setBlockUpdateSignalOnDrag(true);
    connect(m_fadeAspectLocker.data(), SIGNAL(sliderValueChanged()), m_updateCompressor.data(), SLOT(start()));
    connect(m_fadeAspectLocker.data(), SIGNAL(aspectButtonChanged()), m_updateCompressor.data(), SLOT(start()));

    inputSpikes->setRange(2, 20);
    inputSpikes->setValue(2);
    inputSpikes->setBlockUpdateSignalOnDrag(true);
    connect(inputSpikes, SIGNAL(valueChanged(int)), m_updateCompressor.data(), SLOT(start()));

    inputRandomness->setRange(0, 100);
    inputRandomness->setValue(0);
    inputRandomness->setBlockUpdateSignalOnDrag(true);
    connect(inputRandomness, SIGNAL(valueChanged(qreal)), m_updateCompressor.data(), SLOT(start()));

    inputAngle->setRange(0, 360);
    inputAngle->setSuffix(QChar(Qt::Key_degree));
    inputAngle->setValue(0);
    inputAngle->setBlockUpdateSignalOnDrag(true);
    connect(inputAngle, SIGNAL(valueChanged(int)), m_updateCompressor.data(), SLOT(start()));

    connect(spacingWidget, SIGNAL(sigSpacingChanged()), m_updateCompressor.data(), SLOT(start()));

    density->setRange(0, 100, 0);
    density->setSingleStep(1);
    density->setValue(100);
    density->setSuffix("%");
    density->setBlockUpdateSignalOnDrag(true);
    connect(density, SIGNAL(valueChanged(qreal)), m_updateCompressor.data(), SLOT(start()));

    KisCubicCurve topLeftBottomRightLinearCurve;
    topLeftBottomRightLinearCurve.setPoint(0, QPointF(0.0, 1.0));
    topLeftBottomRightLinearCurve.setPoint(1, QPointF(1.0, 0.0));
    softnessCurve->setCurve(topLeftBottomRightLinearCurve);
    connect(softnessCurve, SIGNAL(modified()), m_updateCompressor.data(), SLOT(start()));

    m_brush = QImage(1, 1, QImage::Format_RGB32);

    connect(brushPreview, SIGNAL(clicked()), m_updateCompressor.data(), SLOT(start()));

    QList<KoID> ids = KisMaskGenerator::maskGeneratorIds();
    for (int i = 0; i < ids.size(); i++) {
        comboBoxMaskType->insertItem(i, ids[i].name());
    }

    connect(comboBoxMaskType, SIGNAL(activated(int)), m_updateCompressor.data(), SLOT(start()));
    connect(comboBoxMaskType, SIGNAL(currentIndexChanged(int)), SLOT(setStackedWidget(int)));
    setStackedWidget(comboBoxMaskType->currentIndex());

    brushPreview->setIconSize(QSize(100, 100));

    connect(btnAntialiasing, SIGNAL(toggled(bool)), m_updateCompressor.data(), SLOT(start()));

    m_updateCompressor->start();

}

KisAutoBrushWidget::~KisAutoBrushWidget()
{
}

void KisAutoBrushWidget::resizeEvent(QResizeEvent *)
{
    brushPreview->setMinimumHeight(brushPreview->width()); // dirty hack !
    brushPreview->setMaximumHeight(brushPreview->width()); // dirty hack !
}

void KisAutoBrushWidget::activate()
{
    m_updateCompressor->start();
}

void KisAutoBrushWidget::paramChanged()
{
    KisMaskGenerator* kas;

    bool antialiasEdges = btnAntialiasing->isChecked();

    if (comboBoxMaskType->currentIndex() == 2) { // gaussian brush
        if (comboBoxShape->currentIndex() == 0) {
            kas = new KisGaussCircleMaskGenerator(inputRadius->value(),  inputRatio->value(), inputHFade->value(), inputVFade->value(), inputSpikes->value(), antialiasEdges);
        }
        else {
            kas = new KisGaussRectangleMaskGenerator(inputRadius->value(),  inputRatio->value(), inputHFade->value(), inputVFade->value(), inputSpikes->value(), antialiasEdges);
        }
    }
    else if (comboBoxMaskType->currentIndex() == 1) { // soft brush
        if (comboBoxShape->currentIndex() == 0) {
            kas = new KisCurveCircleMaskGenerator(inputRadius->value(),  inputRatio->value(), inputHFade->value(), inputVFade->value(), inputSpikes->value(), softnessCurve->curve(), antialiasEdges);
        }
        else {
            kas = new KisCurveRectangleMaskGenerator(inputRadius->value(),  inputRatio->value(), inputHFade->value(), inputVFade->value(), inputSpikes->value(), softnessCurve->curve(), antialiasEdges);
        }
    }
    else {// default == 0 or any other
        if (comboBoxShape->currentIndex() == 0) { // use index compare instead of comparing a translatable string
            kas = new KisCircleMaskGenerator(inputRadius->value(),  inputRatio->value(), inputHFade->value(), inputVFade->value(), inputSpikes->value(), antialiasEdges);
        }
        else {
            kas = new KisRectangleMaskGenerator(inputRadius->value(),  inputRatio->value(), inputHFade->value(), inputVFade->value(), inputSpikes->value(), antialiasEdges);
        }
    }
    Q_CHECK_PTR(kas);

    m_autoBrush = new KisAutoBrush(kas, inputAngle->value() / 180.0 * M_PI, inputRandomness->value() / 100.0, density->value() / 100.0);
    m_autoBrush->setSpacing(spacingWidget->spacing());
    m_autoBrush->setAutoSpacing(spacingWidget->autoSpacingActive(), spacingWidget->autoSpacingCoeff());
    m_brush = m_autoBrush->image();

    drawBrushPreviewArea();

    emit sigBrushChanged();
}

void KisAutoBrushWidget::drawBrushPreviewArea() {
    QImage pi(m_brush);
    double coeff = 1.0;
    int bPw = brushPreview->width() - 3;
    if (pi.width() > bPw) {
        coeff =  bPw / (double)pi.width();
    }
    int bPh = brushPreview->height() - 3;
    if (pi.height() > coeff * bPh) {
        coeff = bPh / (double)pi.height();
    }
    if (coeff < 1.0) {
        pi = pi.scaled((int)(coeff * pi.width()) , (int)(coeff * pi.height()),  Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    QPixmap p = QPixmap::fromImage(pi);
    brushPreview->setIcon(QIcon(p));
}

void KisAutoBrushWidget::setStackedWidget(int index)
{
    if (index == 1) {
        stackedWidget->setCurrentIndex(1);
    }
    else {
        stackedWidget->setCurrentIndex(0);
    }
}

KisBrushSP KisAutoBrushWidget::brush()
{
    return m_autoBrush;
}

void KisAutoBrushWidget::setBrush(KisBrushSP brush)
{
    m_autoBrush = brush;
    m_brush = brush->image();
    // XXX: lock, set and unlock the widgets.
    KisAutoBrush* aBrush = dynamic_cast<KisAutoBrush*>(brush.data());

    KisSignalsBlocker b1(comboBoxShape, comboBoxMaskType);
    KisSignalsBlocker b2(inputRadius, inputRatio, inputHFade, inputVFade, inputAngle, inputSpikes);
    KisSignalsBlocker b3(spacingWidget, inputRandomness, density, softnessCurve, btnAntialiasing);

    if (aBrush->maskGenerator()->type() == KisMaskGenerator::CIRCLE) {
        comboBoxShape->setCurrentIndex(0);
    }
    else if (aBrush->maskGenerator()->type() == KisMaskGenerator::RECTANGLE) {
        comboBoxShape->setCurrentIndex(1);
    }
    else {
        comboBoxShape->setCurrentIndex(2);
    }

    const int mastTypeIndex = comboBoxMaskType->findText(aBrush->maskGenerator()->name());
    comboBoxMaskType->setCurrentIndex(mastTypeIndex);
    setStackedWidget(mastTypeIndex); // adjusting manually because the signals are blocked

    inputRadius->setValue(aBrush->maskGenerator()->diameter());
    inputRatio->setValue(aBrush->maskGenerator()->ratio());
    inputHFade->setValue(aBrush->maskGenerator()->horizontalFade());
    inputVFade->setValue(aBrush->maskGenerator()->verticalFade());
    inputAngle->setValue(aBrush->angle() * 180 / M_PI);
    inputSpikes->setValue(aBrush->maskGenerator()->spikes());
    spacingWidget->setSpacing(aBrush->autoSpacingActive(),
                              aBrush->autoSpacingActive() ?
                              aBrush->autoSpacingCoeff() : aBrush->spacing());
    inputRandomness->setValue(aBrush->randomness() * 100);
    density->setValue(aBrush->density() * 100);

    if (!aBrush->maskGenerator()->curveString().isEmpty()) {
        KisCubicCurve curve;
        curve.fromString(aBrush->maskGenerator()->curveString());
        softnessCurve->setCurve(curve);
    }

    btnAntialiasing->setChecked(aBrush->maskGenerator()->antialiasEdges());

    drawBrushPreviewArea(); // sync up what the brush preview area looks like
}


void KisAutoBrushWidget::setBrushSize(qreal dxPixels, qreal dyPixels)
{
    Q_UNUSED(dyPixels);

    qreal newWidth = inputRadius->value() + dxPixels;
    newWidth = qMax(newWidth, qreal(0.1));

    inputRadius->setValue(newWidth);
}

QSizeF KisAutoBrushWidget::brushSize() const
{
    return QSizeF(inputRadius->value(), inputRadius->value() * inputRatio->value());
}

#include "moc_kis_auto_brush_widget.cpp"
