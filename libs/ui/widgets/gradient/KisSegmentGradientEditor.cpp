/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2004 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QPainter>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include <KoColorSpace.h>
#include <resources/KoSegmentGradient.h>

#include "kis_debug.h"

#include "KisSegmentGradientSlider.h"

#include <KoCanvasResourcesIds.h>
#include <KoCanvasResourcesInterface.h>

#include "KisSegmentGradientEditor.h"

/****************************** KisSegmentGradientEditor ******************************/

KisSegmentGradientEditor::KisSegmentGradientEditor(QWidget *parent)
    : QWidget(parent)
    , m_autogradientResource(nullptr)
    , m_canvasResourcesInterface(nullptr)
{
    setupUi(this);

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

    setCompactMode(false);
    setGradient(0);
}

KisSegmentGradientEditor::KisSegmentGradientEditor(KoSegmentGradientSP gradient, QWidget *parent, const char* name, const QString& caption, KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    : KisSegmentGradientEditor(parent)
{
    m_canvasResourcesInterface = canvasResourcesInterface;
    setObjectName(name);
    setWindowTitle(caption);
    setGradient(gradient);
}

void KisSegmentGradientEditor::activate()
{
    paramChanged();
}

void KisSegmentGradientEditor::setCompactMode(bool value)
{
    label->setVisible(!value);
    nameedit->setVisible(!value);
}

void KisSegmentGradientEditor::setGradient(KoSegmentGradientSP gradient)
{
    m_autogradientResource = gradient;
    setEnabled(m_autogradientResource);

    if (m_autogradientResource) {
        gradientSlider->setGradientResource(m_autogradientResource);
        nameedit->setText(m_autogradientResource->name());
        slotSelectedSegment(gradientSlider->selectedSegment());
    }

    emit sigGradientChanged();
}

void KisSegmentGradientEditor::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    m_canvasResourcesInterface = canvasResourcesInterface;
}

KoCanvasResourcesInterfaceSP KisSegmentGradientEditor::canvasResourcesInterface() const
{
    return m_canvasResourcesInterface;
}

void KisSegmentGradientEditor::disableTransparentCheckboxes() {
    leftForegroundTransparent->setEnabled(false);
    leftBackgroundTransparent->setEnabled(false);
    rightForegroundTransparent->setEnabled(false);
    rightBackgroundTransparent->setEnabled(false);
}

void KisSegmentGradientEditor::slotSelectedSegment(KoGradientSegment* segment)
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

void KisSegmentGradientEditor::slotChangedSegment(KoGradientSegment*)
{
    paramChanged();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::slotChangedInterpolation(int type)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment)
        segment->setInterpolation(type);
    gradientSlider->update();

    paramChanged();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::slotChangedColorInterpolation(int type)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment)
        segment->setColorInterpolation(type);
    gradientSlider->update();

    paramChanged();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::slotChangedLeftColor(const KoColor& color)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        KoColor c(color, segment->startColor().colorSpace());
        c.setOpacity(segment->startColor().opacityU8());
        segment->setStartColor(c);
    }
    gradientSlider->update();

    paramChanged();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::slotChangedRightColor(const KoColor& color)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        KoColor c(color, segment->endColor().colorSpace());
        c.setOpacity(segment->endColor().opacityU8());
        segment->setEndColor(c);
    }
    gradientSlider->repaint();

    paramChanged();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::slotChangedLeftOpacity(int value)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        KoColor c(segment->startColor(), segment->startColor().colorSpace());
        c.setOpacity(qreal(value) / qreal(100.0));
        segment->setStartColor(c);
    }
    gradientSlider->repaint();

    paramChanged();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::slotChangedRightOpacity(int value)
{
    KoGradientSegment* segment = gradientSlider->selectedSegment();
    if (segment) {
        KoColor c(segment->endColor(), segment->endColor().colorSpace());
        c.setOpacity(quint8((value *OPACITY_OPAQUE_U8) / 100));
        segment->setEndColor(c);
    }
    gradientSlider->repaint();

    paramChanged();
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::slotChangedLeftType(QAbstractButton* button, bool checked)
{
    if (!checked) { //Radio buttons, so we only care about the one that was checked, not the one unchecked
        return;
    }

    KoGradientSegmentEndpointType type;
    KoColor color;
    const KoColorSpace* colorSpace = m_autogradientResource->colorSpace();

    if (button == leftForegroundRadioButton) {
        if (m_canvasResourcesInterface) {
            color = m_canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>().convertedTo(colorSpace);
        } else {
            color = KoColor(leftColorButton->color(), colorSpace);
        }
        leftForegroundTransparent->setEnabled(true);
        leftBackgroundTransparent->setEnabled(false);
        if (leftForegroundTransparent->isChecked()) {
            type = FOREGROUND_TRANSPARENT_ENDPOINT;
        } else {
            type = FOREGROUND_ENDPOINT;
        }
    } else if (button == leftBackgroundRadioButton) {
        if (m_canvasResourcesInterface) {
            color = m_canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>().convertedTo(colorSpace);
        } else {
            color = KoColor(leftColorButton->color(), colorSpace);
        }
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

void KisSegmentGradientEditor::slotChangedRightType(QAbstractButton* button, bool checked)
{
    if (!checked) { //Radio buttons, so we only care about the one that was checked, not the one unchecked
        return;
    }
    KoGradientSegmentEndpointType type;
    KoColor color;
    const KoColorSpace* colorSpace = m_autogradientResource->colorSpace();
    if (button == rightForegroundRadioButton) {
        if (m_canvasResourcesInterface) {
            color = m_canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>().convertedTo(colorSpace);
        } else {
            color = KoColor(rightColorButton->color(), colorSpace);
        }
        rightForegroundTransparent->setEnabled(true);
        rightBackgroundTransparent->setEnabled(false);
        if (rightForegroundTransparent->isChecked()) {
            type = FOREGROUND_TRANSPARENT_ENDPOINT;
        } else {
            type = FOREGROUND_ENDPOINT;
        }
    } else if (button == rightBackgroundRadioButton) {
        if (m_canvasResourcesInterface) {
            color = m_canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>().convertedTo(colorSpace);
        } else {
            color = KoColor(rightColorButton->color(), colorSpace);
        }
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

void KisSegmentGradientEditor::slotChangedLeftTypeTransparent(bool checked)
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

void KisSegmentGradientEditor::slotChangedRightTypeTransparent(bool checked)
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

void KisSegmentGradientEditor::slotChangedName()
{
    m_autogradientResource->setName(nameedit->text());
    emit sigGradientChanged();
}

void KisSegmentGradientEditor::paramChanged()
{
    m_autogradientResource->updatePreview();
}

