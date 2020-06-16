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

KisAutogradientEditor::KisAutogradientEditor(KoSegmentGradientSP gradient, QWidget *parent, const char* name, const QString& caption, KoColor fgColor, KoColor bgColor)
    : QWidget(parent)
    , m_autogradientResource(gradient)
    , m_fgColor(fgColor)
    , m_bgColor(bgColor)
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

    connect(leftBtnGroup, SIGNAL(buttonToggled(QAbstractButton*, bool)), this, SLOT(slotChangedLeftType(QAbstractButton*, bool)));
    connect(rightBtnGroup, SIGNAL(buttonToggled(QAbstractButton*, bool)), this, SLOT(slotChangedRightType(QAbstractButton*, bool)));
}

void KisAutogradientEditor::activate()
{
    paramChanged();
}

void KisAutogradientEditor::slotSelectedSegment(KoGradientSegment* segment)
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

    KoGradientSegmentEndpointType leftType = segment->startType();
    KoGradientSegmentEndpointType rightType = segment->endType();
    switch (leftType) {
    case COLOR_ENDPOINT:
        leftColorRadioButton->setChecked(true); break;
    case FOREGROUND_ENDPOINT:
    case FOREGROUND_TRANSPARENT_ENDPOINT:
        leftForegroundRadioButton->setChecked(true); break;
    case BACKGROUND_ENDPOINT:
    case BACKGROUND_TRANSPARENT_ENDPOINT:
        leftBackgroundRadioButton->setChecked(true); break;
    }
    switch (rightType) {
    case COLOR_ENDPOINT:
        rightColorRadioButton->setChecked(true); break;
    case FOREGROUND_ENDPOINT:
    case FOREGROUND_TRANSPARENT_ENDPOINT:
        rightForegroundRadioButton->setChecked(true); break;
    case BACKGROUND_ENDPOINT:
    case BACKGROUND_TRANSPARENT_ENDPOINT:
        rightBackgroundRadioButton->setChecked(true); break;
    }


    paramChanged();
}

void KisAutogradientEditor::slotChangedSegment(KoGradientSegment*)
{
    paramChanged();
}

void KisAutogradientEditor::slotChangedInterpolation(int type)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment)
        segment->setInterpolation(type);
    gradientSlider->update();

    paramChanged();
}

void KisAutogradientEditor::slotChangedColorInterpolation(int type)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment)
        segment->setColorInterpolation(type);
    gradientSlider->update();

    paramChanged();
}

void KisAutogradientEditor::slotChangedLeftColor(const KoColor& color)
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

void KisAutogradientEditor::slotChangedRightColor(const KoColor& color)
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

void KisAutogradientEditor::slotChangedLeftOpacity(int value)
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

void KisAutogradientEditor::slotChangedRightOpacity(int value)
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

void KisAutogradientEditor::slotChangedLeftType(QAbstractButton* button, bool checked)
{
    if (!checked) { //Radio buttons, so we only care about the one that was checked, not the one unchecked
        return;
    }
    KoGradientSegmentEndpointType type;
    KoColor color;
    const KoColorSpace* colorSpace = m_autogradientResource->colorSpace();
    if (button == leftForegroundRadioButton) {
        type = FOREGROUND_ENDPOINT;
        color = KoColor(m_fgColor, colorSpace);
    }
    else if (button == leftBackgroundRadioButton) {
        type = BACKGROUND_ENDPOINT;
        color = KoColor(m_bgColor, colorSpace);
    }
    else {
        type = COLOR_ENDPOINT;
        color = KoColor(leftColorButton->color(), colorSpace);
    }
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        segment->setStartType(type);
    }
    slotChangedLeftColor(color);   

}

void KisAutogradientEditor::slotChangedRightType(QAbstractButton* button, bool checked)
{
    if (!checked) { //Radio buttons, so we only care about the one that was checked, not the one unchecked
        return;
    }
    KoGradientSegmentEndpointType type;
    KoColor color;
    const KoColorSpace* colorSpace = m_autogradientResource->colorSpace();
    if (button == rightForegroundRadioButton) {
        type = FOREGROUND_ENDPOINT;
        color = KoColor(m_fgColor, colorSpace);
    }
    else if (button == rightBackgroundRadioButton) {
        type = BACKGROUND_ENDPOINT;
        color = KoColor(m_bgColor, colorSpace);
    }
    else {
        type = COLOR_ENDPOINT;
        color = KoColor(rightColorButton->color(), colorSpace);
    }
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        segment->setEndType(type);
    }
    slotChangedRightColor(color);
}

void KisAutogradientEditor::slotChangedName()
{
    m_autogradientResource->setName(nameedit->text());
}

void KisAutogradientEditor::paramChanged()
{
    m_autogradientResource->updatePreview();
}

