/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include <math.h>

#include <QPainter>
#include <QPixmap>
#include <QLabel>
#include <QComboBox>
#include <q3buttongroup.h>
#include <QPushButton>
#include <qscrollbar.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3Frame>

#include <kdebug.h>

#include "KoChannelInfo.h"
#include "kis_histogram.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_layer.h"
#include "KoColorSpace.h"
#include "kis_histogram_view.h"
#include "kis_basic_histogram_producers.h"
#include "kis_paint_device.h"

KisHistogramView::KisHistogramView(QWidget *parent, const char *name, Qt::WFlags f)
    : QLabel(parent, f)
{
    setObjectName(name);
    // This is needed until we can computationally scale it well. Until then, this is needed
    // And when we have it, it won't hurt to have it around
    setScaledContents(true);
    setFrameShape(Q3Frame::Box); // Draw a box around ourselves
}

KisHistogramView::~KisHistogramView()
{
}

void KisHistogramView::setPaintDevice(KisPaintDeviceSP dev)
{
    m_cs = dev->colorSpace();

    setChannels(); // Sets m_currentProducer to the first in the list

    if (!m_currentProducer)
        return;

    m_from = m_currentProducer->viewFrom();
    m_width = m_currentProducer->viewWidth();

    m_histogram = new KisHistogram(dev, m_currentProducer, LINEAR);

    updateHistogram();
}

void KisHistogramView::setHistogram(KisHistogramSP histogram)
{
    m_cs = 0;
    m_histogram = histogram;
    m_currentProducer = m_histogram->producer();
    m_from = m_currentProducer->viewFrom();
    m_width = m_currentProducer->viewWidth();

    m_comboInfo.clear();
    m_channelStrings.clear();
    m_channels.clear();
    m_channelToOffset.clear();

    addProducerChannels(m_currentProducer);

    // Set the currently viewed channel:
    m_color = false;
    m_channels.append(m_comboInfo.at(1).channel);
    m_channelToOffset.append(0);

    updateHistogram();
}

void KisHistogramView::setView(double from, double size)
{
    m_from = from;
    m_width = size;
    if (m_from + m_width > 1.0)
        m_from = 1.0 - m_width;
    m_histogram->producer()->setView(m_from, m_width);

    m_histogram->updateHistogram();
    updateHistogram();
}

KisHistogramProducerSP KisHistogramView::currentProducer()
{
    return m_currentProducer;
}

QStringList KisHistogramView::channelStrings()
{
    return m_channelStrings;
}

KoIDList KisHistogramView::listProducers()
{
    if (m_cs)
        return KisHistogramProducerFactoryRegistry::instance()->listKeysCompatibleWith(m_cs);
    return KoIDList();
}

void KisHistogramView::setCurrentChannels(const KoID& producerID, Q3ValueVector<KoChannelInfo *> channels)
{
    setCurrentChannels(
        KisHistogramProducerFactoryRegistry::instance()->get(producerID)->generate(),
        channels);
}

void KisHistogramView::setCurrentChannels(KisHistogramProducerSP producer, Q3ValueVector<KoChannelInfo *> channels)
{
    m_currentProducer = producer;
    m_currentProducer->setView(m_from, m_width);
    m_histogram->setProducer(m_currentProducer);
    m_histogram->updateHistogram();
    m_histogram->setChannel(0); // Set a default channel, just being nice

    m_channels.clear();
    m_channelToOffset.clear();

    if (channels.count() == 0) {
        updateHistogram();
        return;
    }

    Q3ValueVector<KoChannelInfo *> producerChannels = m_currentProducer->channels();

    for (int i = 0; i < channels.count(); i++) {
        // Also makes sure the channel is actually in the producer's list
        for (int j = 0; j < producerChannels.count(); j++) {
            if (channels.at(i)->name() == producerChannels.at(j)->name()) {
                m_channelToOffset.append(m_channels.count()); // The first we append maps to 0
                m_channels.append(channels.at(i));
            }
        }
    }

    updateHistogram();
}

bool KisHistogramView::hasColor()
{
    return m_color;
}

void KisHistogramView::setColor(bool set)
{
    if (set != m_color) {
        m_color = set;
        updateHistogram();
    }
}

void KisHistogramView::setActiveChannel(int channel)
{
    ComboboxInfo info = m_comboInfo.at(channel);
    if (info.producer.data() != m_currentProducer.data()) {
        m_currentProducer = info.producer;
        m_currentProducer->setView(m_from, m_width);
        m_histogram->setProducer(m_currentProducer);
        m_histogram->updateHistogram();
    }

    m_channels.clear();
    m_channelToOffset.clear();

    if (!m_currentProducer) {
        updateHistogram();
        return;
    }

    if (info.isProducer) {
        m_color = true;
        m_channels = m_currentProducer->channels();
        for (int i = 0; i < m_channels.count(); i++)
            m_channelToOffset.append(i);
        m_histogram->setChannel(0); // Set a default channel, just being nice
    } else {
        m_color = false;
        Q3ValueVector<KoChannelInfo *> channels = m_currentProducer->channels();
        for (int i = 0; i < channels.count(); i++) {
            KoChannelInfo* channel = channels.at(i);
            if (channel->name() == info.channel->name()) {
                m_channels.append(channel);
                m_channelToOffset.append(i);
                break;
            }
        }
    }

    updateHistogram();
}

void KisHistogramView::setHistogramType(enumHistogramType type)
{
    m_histogram->setHistogramType(type);
    updateHistogram();
}

void KisHistogramView::setChannels()
{
    m_comboInfo.clear();
    m_channelStrings.clear();
    m_channels.clear();
    m_channelToOffset.clear();

    KoIDList list = KisHistogramProducerFactoryRegistry::instance()->listKeysCompatibleWith(m_cs);

    if (list.count() == 0) {
        // XXX: No native histogram for this colorspace. Using converted RGB. We should have a warning
        KisGenericRGBHistogramProducerFactory f;
        addProducerChannels(f.generate());
    } else {
        for (int i = 0; i < list.count(); i++) {
            KoID id(list.at(i));
            addProducerChannels( KisHistogramProducerFactoryRegistry::instance()->get(id)->generate() );
        }
    }

    m_currentProducer = m_comboInfo.at(0).producer;
    m_color = false;
    // The currently displayed channel and its offset
    m_channels.append(m_comboInfo.at(1).channel);
    m_channelToOffset.append(0);
}

void KisHistogramView::addProducerChannels(KisHistogramProducerSP producer) {
        ComboboxInfo info;
        info.isProducer = true;
        info.producer = producer;
        // channel not used for a producer
        Q3ValueVector<KoChannelInfo *> channels = info.producer->channels();
        int count = channels.count();
        m_comboInfo.append(info);
        m_channelStrings.append(producer->id() . name());
        for (int j = 0; j < count; j++) {
            info.isProducer = false;
            info.channel = channels.at(j);
            m_comboInfo.append(info);
            m_channelStrings.append(QString(" ").append(info.channel->name()));
        }
}

void KisHistogramView::updateHistogram()
{
    quint32 height = this->height();
    int selFrom, selTo; // from - to in bins

    if (!m_currentProducer) { // Something's very wrong: no producer for this colorspace to update histogram with!
        return;
    }

    qint32 bins = m_histogram->producer()->numberOfBins();
    m_pix = QPixmap(bins, height);
    m_pix.fill();
    QPainter p(&m_pix);
    p.setBrush(Qt::black);

    // Draw the box of the selection, if any
    if (m_histogram->hasSelection()) {
        double width = m_histogram->selectionTo() - m_histogram->selectionFrom();
        double factor = static_cast<double>(bins) / m_histogram->producer()->viewWidth();
        selFrom = static_cast<int>(m_histogram->selectionFrom() * factor);
        selTo = selFrom + static_cast<int>(width * factor);
        p.drawRect(selFrom, 0, selTo - selFrom, height);
    } else {
        // We don't want the drawing to think we're in a selected area
        selFrom = -1;
        selTo = 2;
    }

    qint32 i = 0;
    double highest = 0;
    bool blackOnBlack = false;

    // First we iterate once, so that we have the overall maximum. This is a bit inefficient,
    // but not too much since the histogram caches the calculations
    for (int chan = 0; chan < m_channels.count(); chan++) {
        m_histogram->setChannel(m_channelToOffset.at(chan));
        if ((double)m_histogram->calculations().getHighest() > highest)
            highest = (double)m_histogram->calculations().getHighest();
    }

    for (int chan = 0; chan < m_channels.count(); chan++) {
        QColor color;
        m_histogram->setChannel(m_channelToOffset.at(chan));

        if (m_color) {
            color = m_channels.at(chan)->color();
            p.setPen(color);
        } else {
            color = Qt::black;
        }
        blackOnBlack = (color == Qt::black);

        if (m_histogram->getHistogramType() == LINEAR) {
            double factor = (double)height / highest;
            for( i=0; i<bins; ++i ) {
                // So that we get a good view even with a selection box with
                // black colors on background of black selection
                if (i >= selFrom && i < selTo && blackOnBlack) {
                    p.setPen(Qt::white);
                } else {
                    p.setPen(color);
                }
                p.drawLine(i, height, i, height - int(m_histogram->getValue(i) * factor));
            }
        } else {
            double factor = (double)height / (double)log(highest);
            for( i = 0; i < bins; ++i ) {
                // Same as above
                if (i >= selFrom && i < selTo && blackOnBlack) {
                    p.setPen(Qt::white);
                } else {
                    p.setPen(color);
                }
                p.drawLine(i, height, i,
                           height - int(log((double)m_histogram->getValue(i)) * factor));
            }
        }
    }

    setPixmap(m_pix);
}

void KisHistogramView::mousePressEvent(QMouseEvent * e) {
    if (e->button() == Qt::RightButton)
        emit rightClicked(e->globalPos());
    else
        QLabel::mousePressEvent(e);
}


#include "kis_histogram_view.moc"
