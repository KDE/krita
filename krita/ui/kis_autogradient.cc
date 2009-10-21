/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *                2004 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_autogradient.h"
#include <QPainter>
#include <QComboBox>

#include <kcolorbutton.h>
#include <knuminput.h>

#include <KoColorSpace.h>
#include <KoSegmentGradient.h>

#include "kis_autogradient_resource.h"
#include "kis_debug.h"

#include "widgets/kis_gradient_slider_widget.h"

/****************************** KisAutogradient ******************************/

KisAutogradient::KisAutogradient(QWidget *parent, const char* name, const QString& caption) : QWidget(parent)
{
    setObjectName(name);
    setupUi(this);
    setWindowTitle(caption);
    m_autogradientResource = new KisAutogradientResource();
    m_autogradientResource->createSegment(INTERP_LINEAR, COLOR_INTERP_RGB, 0.0, 1.0, 0.5, Qt::black, Qt::white);
    gradientSlider->setGradientResource(m_autogradientResource);

    connect(gradientSlider, SIGNAL(sigSelectedSegment(KoGradientSegment*)), SLOT(slotSelectedSegment(KoGradientSegment*)));
    connect(gradientSlider, SIGNAL(sigChangedSegment(KoGradientSegment*)), SLOT(slotChangedSegment(KoGradientSegment*)));
    connect(comboBoxColorInterpolationType, SIGNAL(activated(int)), SLOT(slotChangedColorInterpolation(int)));
    connect(comboBoxInterpolationType, SIGNAL(activated(int)), SLOT(slotChangedInterpolation(int)));
    connect(leftColorButton, SIGNAL(changed(const QColor&)), SLOT(slotChangedLeftColor(const QColor&)));
    connect(rightColorButton, SIGNAL(changed(const QColor&)), SLOT(slotChangedRightColor(const QColor&)));

    connect(intNumInputLeftOpacity, SIGNAL(valueChanged(int)), SLOT(slotChangedLeftOpacity(int)));
    connect(intNumInputRightOpacity, SIGNAL(valueChanged(int)), SLOT(slotChangedRightOpacity(int)));

}

void KisAutogradient::activate()
{
    paramChanged();
}

void KisAutogradient::slotSelectedSegment(KoGradientSegment* segment)
{
    QColor startColor;
    QColor endColor;

    segment->startColor().toQColor(&startColor);
    segment->endColor().toQColor(&endColor);

    leftColorButton->setColor(startColor);
    rightColorButton->setColor(endColor);
    comboBoxColorInterpolationType->setCurrentIndex(segment->colorInterpolation());
    comboBoxInterpolationType->setCurrentIndex(segment->interpolation());

    int leftOpacity = (startColor.alpha() * 100) / OPACITY_OPAQUE;
    intNumInputLeftOpacity->setValue(leftOpacity);

    int rightOpacity = (endColor.alpha() * 100) / OPACITY_OPAQUE;
    intNumInputRightOpacity->setValue(rightOpacity);

    paramChanged();
}

void KisAutogradient::slotChangedSegment(KoGradientSegment*)
{
    paramChanged();
}

void KisAutogradient::slotChangedInterpolation(int type)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment)
        segment->setInterpolation(type);
    gradientSlider->update();

    paramChanged();
}

void KisAutogradient::slotChangedColorInterpolation(int type)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment)
        segment->setColorInterpolation(type);
    gradientSlider->update();

    paramChanged();
}

void KisAutogradient::slotChangedLeftColor(const QColor& color)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        KoColor c(color, segment->startColor().colorSpace());
        c.setOpacity(segment->startColor().opacity());
        segment->setStartColor(c);
    }
    gradientSlider->update();

    paramChanged();
}

void KisAutogradient::slotChangedRightColor(const QColor& color)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        QColor unused;
        KoColor c(color, segment->endColor().colorSpace());
        c.setOpacity(segment->endColor().opacity());
        segment->setEndColor(c);
    }
    gradientSlider->repaint();

    paramChanged();
}

void KisAutogradient::slotChangedLeftOpacity(int value)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        KoColor c(segment->startColor().toQColor(), segment->startColor().colorSpace());
        c.setOpacity((value * OPACITY_OPAQUE) / 100);
        segment->setStartColor(c);
    }
    gradientSlider->repaint();

    paramChanged();
}

void KisAutogradient::slotChangedRightOpacity(int value)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        KoColor c(segment->endColor().toQColor(), segment->endColor().colorSpace());
        c.setOpacity((value *OPACITY_OPAQUE) / 100);
        segment->setEndColor(c);
    }
    gradientSlider->repaint();

    paramChanged();
}

void KisAutogradient::paramChanged()
{
    m_autogradientResource->updatePreview();
    emit activatedResource(m_autogradientResource);
}

#include "kis_autogradient.moc"
