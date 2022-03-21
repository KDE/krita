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
};

KoZoomWidget::KoZoomWidget(QWidget* parent, int maxZoom )
    : QWidget(parent)
    , d(new Private)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    //layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setMargin(0);
    layout->setSpacing(0);

    d->input = new KoZoomInput(this);
    connect(d->input, SIGNAL(zoomLevelChanged(QString)), this, SIGNAL(zoomLevelChanged(QString)));
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
    connect(d->canvasMappingButton, SIGNAL(toggled(bool)), this, SIGNAL(canvasMappingModeChanged(bool)));
    layout->addWidget(d->canvasMappingButton);

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

void KoZoomWidget::setSliderSize(int size)
{
    d->slider->setMaximum(size);
}

void KoZoomWidget::setZoomLevels(const QStringList &values)
{
    d->input->setZoomLevels(values);
}

void KoZoomWidget::setCurrentZoomLevel(const QString &valueString)
{
    d->input->setCurrentZoomLevel(valueString);
}

void KoZoomWidget::setSliderValue(int value)
{
    d->slider->blockSignals(true);
    d->slider->setValue(value);
    d->slider->blockSignals(false);
}

void KoZoomWidget::setCanvasMappingMode(bool status)
{
    if(d->canvasMappingButton && d->canvasMappingButton->isChecked() != status) {
        d->canvasMappingButton->blockSignals(true);
        d->canvasMappingButton->setChecked(status);
        d->canvasMappingButton->blockSignals(false);
    }

    QString canvasMappingMode;

    if (status) {
        d->canvasMappingButton->setIcon(kisIcon("zoom-print"));
        canvasMappingMode = i18n("Print Size");
    } else {
        d->canvasMappingButton->setIcon(kisIcon("zoom-pixels"));
        canvasMappingMode = i18n("Pixel Size");
    }

    d->canvasMappingButton->setToolTip(
                        i18n("Map the displayed canvas size between pixel size or print size\n"
                             "Current Mapping: %1", canvasMappingMode));
}

