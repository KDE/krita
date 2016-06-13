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

#include "kis_histogram_widget.h"

#include <QPainter>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QScrollBar>

#include <kis_debug.h>

#include "KoChannelInfo.h"
#include "KoBasicHistogramProducers.h"
#include "kis_histogram_view.h"

#include "kis_histogram.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_paint_device.h"


KisHistogramWidget::KisHistogramWidget(QWidget *parent, const char *name)
        : WdgHistogram(parent)
{
    setObjectName(name);
    m_from = 0.0;
    m_width = 0.0;
}

KisHistogramWidget::~KisHistogramWidget()
{
}

QList<QString> KisHistogramWidget::producers()
{
    if (m_cs)
        return KoHistogramProducerFactoryRegistry::instance()->keysCompatibleWith(m_cs,true);
    return QList<QString>();
}


void KisHistogramWidget::setChannels(void)
{
    m_comboInfo.clear();
    m_channelStrings.clear();
    m_channels.clear();
    m_channelToOffset.clear();

    QList<QString> list = KoHistogramProducerFactoryRegistry::instance()->keysCompatibleWith(m_cs,false);

    if (list.count() == 0) {
        // XXX: No native histogram for this colorspace. Using converted RGB. We should have a warning
        KoGenericRGBHistogramProducerFactory f;
        addProducerChannels(f.generate());
    } else {
        Q_FOREACH (const QString &id, list) {
            KoHistogramProducer *producer = KoHistogramProducerFactoryRegistry::instance()->value(id)->generate();
            if (producer) {
                addProducerChannels(producer);
            }
        }
    }

    m_currentProducer = m_comboInfo.at(0).producer;
    m_color = false;
    // The currently displayed channel and its offset
    m_channels.append(m_comboInfo.at(1).channel);
    m_channelToOffset.append(0);
}

void KisHistogramWidget::setCurrentChannels(const KoID& producerID, QList<KoChannelInfo *> channels)
{
    setCurrentChannels(
        KoHistogramProducerFactoryRegistry::instance()->value(producerID.id())->generate(),
        channels);
}

void KisHistogramWidget::setCurrentChannels(KoHistogramProducer *producer, QList<KoChannelInfo *> channels)
{
    m_currentProducer = producer;
    m_currentProducer->setView(m_from, m_width);
    m_histogramView->setProducer(m_currentProducer);
    //m_histogram->setChannel(0); // Set a default channel, just being nice

    m_channels.clear();
    m_channelToOffset.clear();

    if (channels.count() == 0) {
        return;
    }

    QList<KoChannelInfo *> producerChannels = m_currentProducer->channels();

    for (int i = 0; i < channels.count(); i++) {
        // Also makes sure the channel is actually in the producer's list
        for (int j = 0; j < producerChannels.count(); j++) {
            if (channels.at(i)->name() == producerChannels.at(j)->name()) {
                m_channelToOffset.append(m_channels.count()); // The first we append maps to 0
                m_channels.append(channels.at(i));
            }
        }
    }
    m_histogramView->setChannels(m_channels);
}

void KisHistogramWidget::addProducerChannels(KoHistogramProducer *producer)
{
    if (!producer) return;

    ComboboxInfo info;
    info.isProducer = true;
    info.producer = producer;
    m_comboInfo.append(info);
    m_channelStrings.append(producer->id().name());
}

void KisHistogramWidget::setActiveChannel(int channel)
{
    ComboboxInfo info = m_comboInfo.at(channel);
    if (info.producer != m_currentProducer) {
        m_currentProducer = info.producer;
        m_currentProducer->setView(m_from, m_width);
        m_histogramView->setProducer(m_currentProducer);
    }

    m_channels.clear();
    m_channelToOffset.clear();

    if (info.isProducer) {
        m_color = true;
        m_channels = m_currentProducer->channels();
        for (int i = 0; i < m_channels.count(); i++)
            m_channelToOffset.append(i);
        //setChannel(0); // Set a default channel, just being nice
    } else {
        m_color = false;
        QList<KoChannelInfo *> channels = m_currentProducer->channels();
        for (int i = 0; i < channels.count(); i++) {
            KoChannelInfo* channel = channels.at(i);
            if (channel->name() == info.channel->name()) {
                m_channels.append(channel);
                m_channelToOffset.append(i);
                break;
            }
        }
    }
    updateEnabled();
}

QStringList KisHistogramWidget::channelStrings()
{
    return m_channelStrings;
}

void KisHistogramWidget::setPaintDevice(KisPaintDeviceSP dev, const QRect &bounds)
{
    radioLinear->disconnect(this);
    radioLog->disconnect(this);
    cmbChannel->disconnect(this);

    m_cs = dev->colorSpace();

    setChannels(); // Sets m_currentProducer to the first in the list
    m_histogramView->setPaintDevice(dev, m_currentProducer, bounds);
    setActiveChannel(0); // So we have the colored one if there are colors

    // The channels
    cmbChannel->clear();
    cmbChannel->addItems(this->channelStrings());
    cmbChannel->setCurrentIndex(0);

    // View display
    currentView->setMinimum(0);
    currentView->setMaximum(100);

    updateEnabled();

    m_from = m_histogramView->currentProducer()->viewFrom();
    m_width = m_histogramView->currentProducer()->viewWidth();

    connect(radioLinear,SIGNAL(clicked()), this, SLOT(slotTypeSwitched()));
    connect(radioLog,SIGNAL(clicked()), this, SLOT(slotTypeSwitched()));
    connect(cmbChannel, SIGNAL(activated(int)), this, SLOT(setActiveChannel(int)));
    connect(zoomIn, SIGNAL(clicked()), this, SLOT(slotZoomIn()));
    connect(zoomOut, SIGNAL(clicked()), this, SLOT(slotZoomOut()));
    connect(currentView, SIGNAL(valueChanged(int)), this, SLOT(slide(int)));
}


void KisHistogramWidget::slotTypeSwitched(void)
{
    m_histogramView->setHistogramType( radioLinear->isChecked()?LINEAR:LOGARITHMIC );
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

void KisHistogramWidget::slotZoomIn()
{
    if ((m_width / 2) >= m_histogramView->currentProducer()->maximalZoom()) {
        setView(m_from, m_width / 2);
    }
}

void KisHistogramWidget::slotZoomOut()
{
    if (m_width * 2 <= 1.0) {
        setView(m_from, m_width * 2);
    }
}

void KisHistogramWidget::slide(int val)
{
    // Beware: at the END (e.g. 100), we want to still view m_width:
    setView((static_cast<double>(val) / 100.0) *(1.0 - m_width), m_width);
}

void KisHistogramWidget::updateEnabled()
{
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


