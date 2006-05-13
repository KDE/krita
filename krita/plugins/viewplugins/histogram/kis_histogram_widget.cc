/*
 *  Copyright (c) 2004 Boudewijn Rempt
 *            (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include <QPainter>
#include <QPixmap>
#include <QLabel>
#include <QComboBox>
#include <q3buttongroup.h>
#include <QPushButton>
#include <qscrollbar.h>

#include <kdebug.h>

#include "kis_channelinfo.h"
#include "kis_histogram_view.h"
#include "kis_histogram_widget.h"
#include "kis_histogram.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_colorspace.h"


KisHistogramWidget::KisHistogramWidget(QWidget *parent, const char *name)
    : super(parent, name)
{
    m_from = 0.0;
    m_width = 0.0;
}

KisHistogramWidget::~KisHistogramWidget()
{
}

void KisHistogramWidget::setPaintDevice(KisPaintDeviceSP dev)
{
    grpType->disconnect(this);
    cmbChannel->disconnect(this);

    m_histogramView->setPaintDevice(dev);
    setActiveChannel(0); // So we have the colored one if there are colors

    // The channels
    cmbChannel->clear();
    cmbChannel->insertStringList(m_histogramView->channelStrings());
    cmbChannel->setCurrentItem(0);

    // View display
    currentView->setMinValue(0);
    currentView->setMaxValue(100);

    updateEnabled();

    m_from = m_histogramView->currentProducer()->viewFrom();
    m_width = m_histogramView->currentProducer()->viewWidth();

    connect(grpType, SIGNAL(clicked(int)), this, SLOT(slotTypeSwitched(int)));
    connect(cmbChannel, SIGNAL(activated(int)), this, SLOT(setActiveChannel(int)));
    connect(zoomIn, SIGNAL(clicked()), this, SLOT(slotZoomIn()));
    connect(zoomOut, SIGNAL(clicked()), this, SLOT(slotZoomOut()));
    connect(currentView, SIGNAL(valueChanged(int)), this, SLOT(slide(int)));
}

void KisHistogramWidget::setActiveChannel(int channel)
{
    m_histogramView->setActiveChannel(channel);
    updateEnabled();
}

void KisHistogramWidget::slotTypeSwitched(int id)
{
    if (id == LINEAR)
        m_histogramView->setHistogramType(LINEAR);
    else if (id == LOGARITHMIC)
        m_histogramView->setHistogramType(LOGARITHMIC);
}

void KisHistogramWidget::setView(double from, double size)
{
    m_from = from;
    m_width = size;
    if (m_from + m_width > 1.0)
        m_from = 1.0 - m_width;
    m_histogramView->setView(m_from, m_width);
    updateEnabled();
}

void KisHistogramWidget::slotZoomIn() {
    if ((m_width / 2) >= m_histogramView->currentProducer()->maximalZoom()) {
        setView(m_from, m_width / 2);
    }
}

void KisHistogramWidget::slotZoomOut() {
    if (m_width * 2 <= 1.0) {
        setView(m_from, m_width * 2);
    }
}

void KisHistogramWidget::slide(int val) {
    // Beware: at the END (e.g. 100), we want to still view m_width:
    setView((static_cast<double>(val) / 100.0) * (1.0 - m_width), m_width);
}

void KisHistogramWidget::updateEnabled() {
    if (m_histogramView->currentProducer()->maximalZoom() < 1.0) {
        if ((m_width / 2) >= m_histogramView->currentProducer()->maximalZoom()) {
            zoomIn->setEnabled(true);
        } else {
            zoomIn->setEnabled(false);
        }
        if (m_width * 2 <= 1.0) {
            zoomOut->setEnabled(true);
        } else {
            zoomOut->setEnabled(false);
        }
        if (m_width < 1.0)
            currentView->setEnabled(true);
        else
            currentView->setEnabled(false);
    } else {
        zoomIn->setEnabled(false);
        zoomOut->setEnabled(false);
        currentView->setEnabled(false);
    }
}

#include "kis_histogram_widget.moc"

