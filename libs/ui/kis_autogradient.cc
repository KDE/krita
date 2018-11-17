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
#include <QSpinBox>
#include <QDoubleSpinBox>

#include <KoColorSpace.h>
#include <resources/KoSegmentGradient.h>

#include "kis_debug.h"

#include "KisGradientSliderWidget.h"

/****************************** KisAutogradient ******************************/

KisAutogradient::KisAutogradient(KoSegmentGradientSP gradient, QWidget *parent, const char* name, const QString& caption)
    : QWidget(parent)
    , m_autogradientResource(gradient)
{
    setObjectName(name);
    setupUi(this);
    setWindowTitle(caption);
    gradientSlider->setGradientResource(m_autogradientResource);
    nameedit->setText(gradient->name());
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        slotSelectedSegment(segment);
    }

    connect(nameedit, SIGNAL(editingFinished()), this, SLOT(slotChangedName()));
    connect(gradientSlider, SIGNAL(sigSelectedSegment(KoGradientSegment*)), SLOT(slotSelectedSegment(KoGradientSegment*)));
    connect(gradientSlider, SIGNAL(sigChangedSegment(KoGradientSegment*)), SLOT(slotChangedSegment(KoGradientSegment*)));
    connect(comboBoxColorInterpolationType, SIGNAL(activated(int)), SLOT(slotChangedColorInterpolation(int)));
    connect(comboBoxInterpolationType, SIGNAL(activated(int)), SLOT(slotChangedInterpolation(int)));
    connect(leftColorButton, SIGNAL(changed(KoColor)), SLOT(slotChangedLeftColor(KoColor)));
    connect(rightColorButton, SIGNAL(changed(KoColor)), SLOT(slotChangedRightColor(KoColor)));

    connect(intNumInputLeftOpacity, SIGNAL(valueChanged(int)), SLOT(slotChangedLeftOpacity(int)));
    connect(intNumInputRightOpacity, SIGNAL(valueChanged(int)), SLOT(slotChangedRightOpacity(int)));

}

void KisAutogradient::activate()
{
    paramChanged();
}

void KisAutogradient::slotSelectedSegment(KoGradientSegment* segment)
{

    leftColorButton->setColor(segment->startColor());
    rightColorButton->setColor(segment->endColor());
    comboBoxColorInterpolationType->setCurrentIndex(segment->colorInterpolation());
    comboBoxInterpolationType->setCurrentIndex(segment->interpolation());

    int leftOpacity = segment->startColor().opacityF();
    intNumInputLeftOpacity->setValue(leftOpacity * 100);
    intNumInputLeftOpacity->setSuffix(i18n(" %"));

    int rightOpacity = segment->endColor().opacityF();
    intNumInputRightOpacity->setValue(rightOpacity * 100);
    intNumInputRightOpacity->setSuffix(i18n(" %"));

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

void KisAutogradient::slotChangedLeftColor(const KoColor& color)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        KoColor c(color, segment->startColor().colorSpace());
        c.setOpacity(segment->startColor().opacityU8());
        segment->setStartColor(c);
    }
    gradientSlider->update();

    paramChanged();
}

void KisAutogradient::slotChangedRightColor(const KoColor& color)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        KoColor c(color, segment->endColor().colorSpace());
        c.setOpacity(segment->endColor().opacityU8());
        segment->setEndColor(c);
    }
    gradientSlider->repaint();

    paramChanged();
}

void KisAutogradient::slotChangedLeftOpacity(int value)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        KoColor c(segment->startColor(), segment->startColor().colorSpace());
        c.setOpacity(qreal(value) / qreal(100.0));
        segment->setStartColor(c);
    }
    gradientSlider->repaint();

    paramChanged();
}

void KisAutogradient::slotChangedRightOpacity(int value)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        KoColor c(segment->endColor(), segment->endColor().colorSpace());
        c.setOpacity(quint8((value *OPACITY_OPAQUE_U8) / 100));
        segment->setEndColor(c);
    }
    gradientSlider->repaint();

    paramChanged();
}

void KisAutogradient::slotChangedName()
{
    m_autogradientResource->setName(nameedit->text());
}

void KisAutogradient::paramChanged()
{
    m_autogradientResource->updatePreview();
}

