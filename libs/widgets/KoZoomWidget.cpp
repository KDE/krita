/*  
    Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>
    Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>
    Copyright (C) 2006-2007 C. Boemann <cbo@boemann.dk>
    Copyright (C) 2014 Sven Langkamp <sven.langkamp@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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

    qreal effectiveZoom;
};

KoZoomWidget::KoZoomWidget(QWidget* parent, KoZoomAction::SpecialButtons specialButtons, int maxZoom )
    : QWidget(parent)
    , d(new Private)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    //layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setMargin(0);
    layout->setSpacing(0);

    d->input = new KoZoomInput(this);
    connect(d->input, SIGNAL(zoomLevelChanged(const QString&)), this, SIGNAL(zoomLevelChanged(const QString&)));
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

    if (specialButtons & KoZoomAction::AspectMode) {
        d->aspectButton = new QToolButton(this);
        d->aspectButton->setIcon(koIcon("zoom-pixels"));
        d->aspectButton->setIconSize(QSize(16,16));
        d->aspectButton->setCheckable(true);
        d->aspectButton->setChecked(true);
        d->aspectButton->setAutoRaise(true);
        d->aspectButton->setToolTip(i18n("Use same aspect as pixels"));
        connect(d->aspectButton, SIGNAL(toggled(bool)), this, SIGNAL(aspectModeChanged(bool)));
        layout->addWidget(d->aspectButton);
    }
    if (specialButtons & KoZoomAction::ZoomToSelection) {
        QToolButton * zoomToSelectionButton = new QToolButton(this);
        zoomToSelectionButton->setIcon(koIcon("zoom-select"));
        zoomToSelectionButton->setIconSize(QSize(16,16));
        zoomToSelectionButton->setAutoRaise(true);
        zoomToSelectionButton->setToolTip(i18n("Zoom to Selection"));
        connect(zoomToSelectionButton, SIGNAL(clicked(bool)), this, SIGNAL(zoomedToSelection()));
        layout->addWidget(zoomToSelectionButton);
    }
    if (specialButtons & KoZoomAction::ZoomToAll) {
        QToolButton * zoomToAllButton = new QToolButton(this);
        zoomToAllButton->setIcon(koIcon("zoom-draw"));
        zoomToAllButton->setIconSize(QSize(16,16));
        zoomToAllButton->setAutoRaise(true);
        zoomToAllButton->setToolTip(i18n("Zoom to All"));
        connect(zoomToAllButton, SIGNAL(clicked(bool)), this, SIGNAL(zoomedToAll()));
        layout->addWidget(zoomToAllButton);
    }
    connect(d->slider, SIGNAL(valueChanged(int)), this, SIGNAL(sliderValueChanged(int)));
}

KoZoomWidget::~KoZoomWidget()
{
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
