/*
 *  Copyright (c) 2004,2007,2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_auto_brush_widget.h"
#include <kis_debug.h>
#include <QSpinBox>
#include <QToolButton>
#include <QImage>
#include <QComboBox>
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

#include "kis_mask_generator.h"

#define showSlider(input, step) input->setRange(input->minimum(), input->maximum(), step)

KisAutoBrushWidget::KisAutoBrushWidget(QWidget *parent, const char* name, const QString& caption)
        : KisWdgAutobrush(parent, name)
        , m_autoBrush(0)
{
    setWindowTitle(caption);

    m_linkSize = true;
    m_linkFade = true;

//     linkFadeToggled(m_linkSize);
//     linkSizeToggled(m_linkFade);

    connect(aspectButton, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(linkFadeToggled(bool)));
    aspectButton->setKeepAspectRatio(m_linkFade);

    connect((QObject*)comboBoxShape, SIGNAL(activated(int)), this, SLOT(paramChanged()));

//     showSlider(inputRadius);
    inputRadius->setRange(inputRadius->minimum(), inputRadius->maximum(), 1.0, false);
    connect(inputRadius, SIGNAL(valueChanged(double)), this, SLOT(spinBoxRadiusChanged(double)));

    showSlider(inputRatio, 0.1);
    connect(inputRatio, SIGNAL(valueChanged(double)), this, SLOT(spinBoxRatioChanged(double)));

    showSlider(inputHFade, 0.1);
    connect(inputHFade, SIGNAL(valueChanged(double)), this, SLOT(spinBoxHorizontalChanged(double)));

    showSlider(inputVFade, 0.1);
    connect(inputVFade, SIGNAL(valueChanged(double)), this, SLOT(spinBoxVerticalChanged(double)));

    inputSpikes->setSliderEnabled(true);
    connect(inputSpikes, SIGNAL(valueChanged(int)), this, SLOT(paramChanged()));

    inputAngle->setSliderEnabled(true);
    connect(inputAngle, SIGNAL(valueChanged(int)), this, SLOT(paramChanged()));

    showSlider(inputSpacing, 0.1);
    connect(inputSpacing, SIGNAL(valueChanged(double)), this, SLOT(paramChanged()));

    m_brush = QImage(1, 1, QImage::Format_RGB32);

    connect(brushPreview, SIGNAL(clicked()), SLOT(paramChanged()));

    brushPreview->setIconSize(QSize(100, 100));

    paramChanged();

}

void KisAutoBrushWidget::resizeEvent(QResizeEvent *)
{
    brushPreview->setMinimumHeight(brushPreview->width()); // dirty hack !
    brushPreview->setMaximumHeight(brushPreview->width()); // dirty hack !
}

void KisAutoBrushWidget::activate()
{
    paramChanged();
}

void KisAutoBrushWidget::paramChanged()
{
    KisMaskGenerator* kas;

    if (comboBoxShape->currentIndex() == 0) { // use index compare instead of comparing a translatable string
        kas = new KisCircleMaskGenerator(inputRadius->value(),  inputRatio->value(), inputHFade->value(), inputVFade->value(), inputSpikes->value());
        Q_CHECK_PTR(kas);

    } else {
        kas = new KisRectangleMaskGenerator(inputRadius->value(),  inputRatio->value(), inputHFade->value(), inputVFade->value(), inputSpikes->value());
        Q_CHECK_PTR(kas);

    }
    m_autoBrush = new KisAutoBrush(kas, inputAngle->value() / 180.0 * M_PI);
    m_autoBrush->setSpacing(inputSpacing->value());
    m_brush = m_autoBrush->image();

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

    emit sigBrushChanged();
}

void KisAutoBrushWidget::spinBoxRatioChanged(double a)
{
    Q_UNUSED(a);
    paramChanged();
}
void KisAutoBrushWidget::spinBoxRadiusChanged(double a)
{
    Q_UNUSED(a);
    paramChanged();
}
void KisAutoBrushWidget::spinBoxHorizontalChanged(double a)
{
    if (m_linkFade)
        inputVFade->setValue(a);
    paramChanged();
}
void KisAutoBrushWidget::spinBoxVerticalChanged(double a)
{
    if (m_linkFade)
        inputHFade->setValue(a);
    paramChanged();
}

void KisAutoBrushWidget::linkFadeToggled(bool b)
{
    m_linkFade = b;

    if (m_linkFade)
        inputVFade->setValue(inputHFade->value());
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
    inputRadius->setValue(aBrush->maskGenerator()->radius());
    inputRatio->setValue(aBrush->maskGenerator()->ratio());
    inputHFade->setValue(aBrush->maskGenerator()->horizontalFade());
    inputVFade->setValue(aBrush->maskGenerator()->verticalFade());
    inputAngle->setValue(aBrush->angle() * 180 / M_PI);
    inputSpikes->setValue(aBrush->maskGenerator()->spikes());
    inputSpacing->setValue(aBrush->spacing());
}


void KisAutoBrushWidget::setAutoBrushDiameter(qreal diameter)
{
    inputRadius->setValue(diameter);
}


qreal KisAutoBrushWidget::autoBrushDiameter()
{
    return inputRadius->value();
}



#include "kis_auto_brush_widget.moc"
