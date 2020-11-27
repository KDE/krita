/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *                2004 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

#include <KoCanvasResourcesIds.h>
#include <KoCanvasResourcesInterface.h>

/****************************** KisAutogradient ******************************/

KisAutogradientEditor::KisAutogradientEditor(KoSegmentGradientSP gradient, QWidget *parent, const char* name, const QString& caption, KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    : QWidget(parent)
    , m_autogradientResource(gradient)
    , m_canvasResourcesInterface(canvasResourcesInterface)
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

    connect(leftForegroundTransparent, SIGNAL(toggled(bool)), this, SLOT(slotChangedLeftTypeTransparent(bool)));
    connect(leftBackgroundTransparent, SIGNAL(toggled(bool)), this, SLOT(slotChangedLeftTypeTransparent(bool)));
    connect(rightForegroundTransparent, SIGNAL(toggled(bool)), this, SLOT(slotChangedRightTypeTransparent(bool)));
    connect(rightBackgroundTransparent, SIGNAL(toggled(bool)), this, SLOT(slotChangedRightTypeTransparent(bool)));
}

void KisAutogradientEditor::activate()
{
    paramChanged();
}

void KisAutogradientEditor::disableTransparentCheckboxes() {
    leftForegroundTransparent->setEnabled(false);
    leftBackgroundTransparent->setEnabled(false);
    rightForegroundTransparent->setEnabled(false);
    rightBackgroundTransparent->setEnabled(false);
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
    disableTransparentCheckboxes(); //disable all of them, then enable the correct ones
    switch (leftType) {
    case COLOR_ENDPOINT:
        leftColorRadioButton->setChecked(true); break;
    case FOREGROUND_TRANSPARENT_ENDPOINT:
        leftForegroundTransparent->setChecked(true);
        Q_FALLTHROUGH();
    case FOREGROUND_ENDPOINT:
        leftForegroundTransparent->setEnabled(true);
        leftForegroundRadioButton->setChecked(true); break;
    case BACKGROUND_TRANSPARENT_ENDPOINT:
        leftBackgroundTransparent->setChecked(true);
        Q_FALLTHROUGH();
    case BACKGROUND_ENDPOINT:
        leftBackgroundTransparent->setEnabled(true);
        leftBackgroundRadioButton->setChecked(true); break;
    }
    switch (rightType) {
    case COLOR_ENDPOINT:
        rightColorRadioButton->setChecked(true); break;
    case FOREGROUND_TRANSPARENT_ENDPOINT:
        rightForegroundTransparent->setChecked(true);
        Q_FALLTHROUGH();
    case FOREGROUND_ENDPOINT:
        rightForegroundTransparent->setEnabled(true);
        rightForegroundRadioButton->setChecked(true); break;
    case BACKGROUND_TRANSPARENT_ENDPOINT:
        rightBackgroundTransparent->setChecked(true);
        Q_FALLTHROUGH();
    case BACKGROUND_ENDPOINT:
        rightBackgroundTransparent->setEnabled(true);
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
        color = m_canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>().convertedTo(colorSpace);
        leftForegroundTransparent->setEnabled(true);
        leftBackgroundTransparent->setEnabled(false);
        if (leftForegroundTransparent->isChecked()) {
            type = FOREGROUND_TRANSPARENT_ENDPOINT;
        } else {
            type = FOREGROUND_ENDPOINT;
        }
    } else if (button == leftBackgroundRadioButton) {
        color = m_canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>().convertedTo(colorSpace);
        leftBackgroundTransparent->setEnabled(true);
        leftForegroundTransparent->setEnabled(false);
        if (leftBackgroundTransparent->isChecked()) {
            type = BACKGROUND_TRANSPARENT_ENDPOINT;
        } else {
            type = BACKGROUND_ENDPOINT;
        }
    }
    else {
        type = COLOR_ENDPOINT;
        leftForegroundTransparent->setEnabled(false);
        leftBackgroundTransparent->setEnabled(false);
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
        color = m_canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>().convertedTo(colorSpace);
        rightForegroundTransparent->setEnabled(true);
        rightBackgroundTransparent->setEnabled(false);
        if (rightForegroundTransparent->isChecked()) {
            type = FOREGROUND_TRANSPARENT_ENDPOINT;
        } else {
            type = FOREGROUND_ENDPOINT;
        }
    } else if (button == rightBackgroundRadioButton) {
        color = m_canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>().convertedTo(colorSpace);
        rightBackgroundTransparent->setEnabled(true);
        rightForegroundTransparent->setEnabled(false);
        if (rightBackgroundTransparent->isChecked()) {
            type = BACKGROUND_TRANSPARENT_ENDPOINT;
        } else {
            type = BACKGROUND_ENDPOINT;
        }
    }
    else {
        type = COLOR_ENDPOINT;
        rightForegroundTransparent->setEnabled(false);
        rightBackgroundTransparent->setEnabled(false);
        color = KoColor(rightColorButton->color(), colorSpace);
    }
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        segment->setEndType(type);
    }
    slotChangedRightColor(color);
}

void KisAutogradientEditor::slotChangedLeftTypeTransparent(bool checked)
{
    if (leftColorRadioButton->isChecked()) { //shouldn't be able to check/uncheck in this state, but just in case
        return;
    }

    KoGradientSegmentEndpointType type;
    if (leftForegroundRadioButton->isChecked()) {
        if (checked) {
            type = FOREGROUND_TRANSPARENT_ENDPOINT;
        } else {
            type = FOREGROUND_ENDPOINT;
        }
    } else { //leftBackgroundRadioButton is checked
        if (checked) {
            type = BACKGROUND_TRANSPARENT_ENDPOINT;
        } else {
            type = BACKGROUND_ENDPOINT;
        }
    }

    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        segment->setStartType(type);
        slotChangedLeftColor(segment->startColor());
    }
}

void KisAutogradientEditor::slotChangedRightTypeTransparent(bool checked)
{
    if (rightColorRadioButton->isChecked()) { //shouldn't be able to check/uncheck in this state, but just in case
        return;
    }

    KoGradientSegmentEndpointType type;
    if (rightForegroundRadioButton->isChecked()) {
        if (checked) {
            type = FOREGROUND_TRANSPARENT_ENDPOINT;
        } else {
            type = FOREGROUND_ENDPOINT;
        }
    } else { //rightBackgroundRadioButton is checked
        if (checked) {
            type = BACKGROUND_TRANSPARENT_ENDPOINT;
        } else {
            type = BACKGROUND_ENDPOINT;
        }
    }

    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        segment->setEndType(type);
        slotChangedRightColor(segment->endColor());
    }
}

void KisAutogradientEditor::slotChangedName()
{
    m_autogradientResource->setName(nameedit->text());
}

void KisAutogradientEditor::paramChanged()
{
    m_autogradientResource->updatePreview();
}

