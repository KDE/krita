/*  
    SPDX-FileCopyrightText: 2004 Ariya Hidayat <ariya@kde.org>
    SPDX-FileCopyrightText: 2006 Peter Simonsson <peter.simonsson@gmail.com>
    SPDX-FileCopyrightText: 2006-2007 C. Boemann <cbo@boemann.dk>
    SPDX-FileCopyrightText: 2014 Sven Langkamp <sven.langkamp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "KoZoomWidget.h"

#include <QSlider>
#include <QToolButton>
#include <QBoxLayout>
#include <QDebug>
#include <klocalizedstring.h>

#include "KoZoomInput.h"
#include "KoIcon.h"

class KoZoomWidget::Private
{
public:

    Private()
        : slider(0)
        , input(0)
        , canvasMappingButton(0)
    {}

    QSlider* slider;
    KoZoomInput* input;
    QToolButton* canvasMappingButton;

    qreal effectiveZoom {1.0};

    void updateUsePrintResolutionButtonIcon(bool value);
};

KoZoomWidget::KoZoomWidget(QWidget* parent, int maxZoom )
    : QWidget(parent)
    , d(new Private)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    //layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    d->input = new KoZoomInput(this);
    connect(d->input, SIGNAL(zoomLevelChanged(QString)), this, SIGNAL(zoomLevelChanged(QString)));
    connect(d->input, SIGNAL(zoomLevelChangedIndex(int)), this, SIGNAL(zoomLevelChangedIndex(int)));
    layout->addWidget(d->input);

    d->slider = new QSlider(Qt::Horizontal);
    d->slider->setToolTip(i18n("Zoom"));
    d->slider->setMinimum(0);
    d->slider->setMaximum(maxZoom);
    d->slider->setValue(0);
    d->slider->setSingleStep(1);
    d->slider->setPageStep(1);
    d->slider->setMinimumWidth(80);
    layout->addWidget(d->slider);
    layout->setStretch(1, 1);

    d->canvasMappingButton = new QToolButton(this);
    d->canvasMappingButton->setIcon(kisIcon("zoom-pixels"));
    d->canvasMappingButton->setCheckable(true);
    d->canvasMappingButton->setChecked(false);
    d->canvasMappingButton->setAutoRaise(true);
    layout->addWidget(d->canvasMappingButton);
    connect(d->canvasMappingButton, &QToolButton::toggled, this, [this](bool value) {
        d->updateUsePrintResolutionButtonIcon(value);
        Q_EMIT sigUsePrintResolutionModeChanged(value);
    });
    d->updateUsePrintResolutionButtonIcon(false);

    connect(d->slider, SIGNAL(valueChanged(int)), this, SIGNAL(sliderValueChanged(int)));
}

KoZoomWidget::~KoZoomWidget()
{
}

bool KoZoomWidget::isZoomInputFlat() const
{
    return d->input->isFlat();
}

void KoZoomWidget::setZoomInputFlat(bool flat)
{
    d->input->setFlat(flat);
}

void KoZoomWidget::setSliderState(int size, int index)
{
    QSignalBlocker b(d->slider);
    d->slider->setMaximum(size - 1);
    d->slider->setValue(index);
}

void KoZoomWidget::setZoomLevelsState(const QStringList &values, int index, const QString &activeText)
{
    QSignalBlocker b(d->input);
    d->input->setZoomLevels(values);
    d->input->setCurrentZoomLevel(index, activeText);
}

void KoZoomWidget::setCurrentZoomLevel(int index)
{
    d->input->setCurrentZoomLevel(index);
}

void KoZoomWidget::setCurrentZoomLevel(const QString &valueString)
{
    d->input->setCurrentZoomLevel(valueString);
}

void KoZoomWidget::setSliderValue(int value)
{
    QSignalBlocker b(d->slider);
    d->slider->setValue(value);
}

void KoZoomWidget::Private::updateUsePrintResolutionButtonIcon(bool value)
{
    QString canvasMappingMode;

    if (value) {
        canvasMappingButton->setIcon(kisIcon("zoom-print"));
        canvasMappingMode = i18n("Print Size");
    } else {
        canvasMappingButton->setIcon(kisIcon("zoom-pixels"));
        canvasMappingMode = i18n("Pixel Size");
    }

    canvasMappingButton->setToolTip(
        i18n("Map the displayed canvas size between pixel size or print size\n"
             "Current Mapping: %1",
             canvasMappingMode));
}

void KoZoomWidget::setUsePrintResolutionMode(bool value)
{
    if (d->canvasMappingButton->isChecked() != value) {
        QSignalBlocker b(d->canvasMappingButton);
        d->canvasMappingButton->setChecked(value);
        d->updateUsePrintResolutionButtonIcon(value);
    }
}

