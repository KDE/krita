/*
 * Copyright (C) Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>, (C) 2016
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

#include <QList>
#include <QAbstractSpinBox>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include "KoColorSpaceRegistry.h"

#include "kis_signal_compressor.h"
#include "kis_canvas_resource_provider.h"
#include "KoColorDisplayRendererInterface.h"
#include "kis_display_color_converter.h"
#include "kis_spinbox_color_selector.h"

#include "kis_internal_color_selector.h"

struct KisInternalColorSelector::Private
{
    bool allowUpdates = true;
    KoColor currentColor;
    const KoColorSpace *currentColorSpace;

    KisCanvas2 *canvas;

    KisSpinboxColorSelector *spinBoxSelector;
    KisSignalCompressor *compressColorChanges;
};

KisInternalColorSelector::KisInternalColorSelector(QWidget *parent, const QString &caption)
    : QDialog(parent)
     ,m_d(new Private)
{
    setModal(false);
    setWindowTitle(caption);

    m_d->currentColor = KoColor();
    m_d->currentColorSpace = m_d->currentColor.colorSpace();
    m_d->spinBoxSelector = new KisSpinboxColorSelector;
    m_d->spinBoxSelector->slotSetColorSpace(m_d->currentColorSpace);
    ui->base->addWidget(m_d->spinBoxSelector);
    connect(m_d->spinBoxSelector, SIGNAL(sigNewColor(KoColor)), this, SLOT(slotColorUpdated(KoColor)));

    connect(this, SIGNAL(signalForegroundColorChosen(KoColor)), this, SLOT(slotLockSelector()));
    m_d->compressColorChanges = new KisSignalCompressor(500 /* ms */, KisSignalCompressor::POSTPONE, this);
    connect(m_d->compressColorChanges, SIGNAL(timeout()), this, SLOT(sentUpdateWithNewColor()));
}

KisInternalColorSelector::~KisInternalColorSelector()
{
    delete ui;
    //TODO: Does the scoped pointer also need to be deleted???
}

void KisInternalColorSelector::setCanvas(KisCanvas2 *canvas)
{
    if (m_d->canvas) {
        m_d->canvas->disconnectCanvasObserver(this);
    }
    m_d->canvas = canvas;

    if (m_d->canvas) {
        //basics//
        connect(m_d->canvas->imageView()->resourceProvider(), SIGNAL(sigFGColorChanged(KoColor)), this, SLOT(slotColorUpdated(KoColor)));
        connect(this, SIGNAL(signalForegroundColorChosen(KoColor)), m_d->canvas->imageView()->resourceProvider(), SLOT(slotSetFGColor(KoColor)));
        //display color converter//
        connect(m_d->canvas->displayColorConverter(), SIGNAL(displayConfigurationChanged()), this, SLOT(slotConfigurationChanged()));
    }
}

void KisInternalColorSelector::unsetCanvas()
{
    setEnabled(false);
    m_d->canvas = 0;
}

void KisInternalColorSelector::slotColorUpdated(KoColor newColor)
{
    //if the update did not come from this selector...
    if (m_d->allowUpdates) {
        m_d->currentColor = newColor;
        updateAllElements();
    } else {
        emit(signalForegroundColorChosen(m_d->currentColor));
        m_d->compressColorChanges->start();
    }
}

void KisInternalColorSelector::slotColorSpaceChanged(const KoColorSpace *cs)
{
    if (cs == m_d->currentColorSpace) {
        return;
    }

    m_d->currentColorSpace = KoColorSpaceRegistry::instance()->colorSpace(cs->colorModelId().id(), cs->colorDepthId().id(), cs->profile());
    //Empty the layout.
    m_d->spinBoxSelector->slotSetColorSpace(m_d->currentColorSpace);

}

void KisInternalColorSelector::slotConfigurationChanged()
{
    if (!m_d->canvas) {
        return;
    }
    //m_d->canvas->displayColorConverter()->
    //slotColorSpaceChanged(m_d->canvas->image()->colorSpace());
}

void KisInternalColorSelector::slotLockSelector()
{
    m_d->allowUpdates = false;
}

void KisInternalColorSelector::updateAllElements()
{
    //update everything!!!
    m_d->spinBoxSelector->slotSetColor(m_d->currentColor);
}

void KisInternalColorSelector::endUpdateWithNewColor()
{
    m_d->allowUpdates = true;
}
