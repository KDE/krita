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

#include <klocalizedstring.h>

#include "KoZoomInput.h"
#include "KoIcon.h"

class KoZoomWidget::Private
{
public:

    Private()
        : slider(0)
        , input(0)
        , aspectButton(0)
    {}

    QSlider* slider;
    KoZoomInput* input;
    QToolButton* aspectButton;

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

    d->aspectButton = new QToolButton(this);
    d->aspectButton->setIcon(kisIcon("zoom-pixels"));
    d->aspectButton->setCheckable(true);
    d->aspectButton->setChecked(true);
    d->aspectButton->setAutoRaise(true);
    d->aspectButton->setToolTip(i18n("Use same aspect as pixels"));
    connect(d->aspectButton, SIGNAL(toggled(bool)), this, SIGNAL(aspectModeChanged(bool)));
    layout->addWidget(d->aspectButton);

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

void KoZoomWidget::setAspectMode(bool status)
{
    if(d->aspectButton && d->aspectButton->isChecked() != status) {
        d->aspectButton->blockSignals(true);
        d->aspectButton->setChecked(status);
        d->aspectButton->blockSignals(false);
    }
}
