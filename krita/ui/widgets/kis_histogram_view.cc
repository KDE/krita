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

#include <qpainter.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qscrollbar.h>

#include <kdebug.h>

#include "kis_channelinfo.h"
#include "kis_histogram.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_abstract_colorspace.h"
#include "kis_histogram_view.h"
#include "kis_basic_histogram_producers.h"


KisHistogramView::KisHistogramView(QWidget *parent, const char *name, WFlags f) 
    : QLabel(parent, name, f)
{
    // This is needed until we can computationally scale it well. Until then, this is needed
    // And when we have it, it won't hurt to have it around
    setScaledContents(true);
    setFrameShape(QFrame::Box); // Draw a box around ourselves
}

KisHistogramView::~KisHistogramView()
{
}

void KisHistogramView::setLayer(KisLayerSP layer)
{
    m_cs = layer -> colorSpace();

    setChannels(); // Sets m_currentProducer to the first in the list

    if (!m_currentProducer)
        return;

    kdDebug() << "Histogram using the Histogram Producer "
              << m_currentProducer -> id().name() << endl;

    m_from = m_currentProducer -> viewFrom();
    m_width = m_currentProducer -> viewWidth();

    m_histogram = new KisHistogram(layer, m_currentProducer, LINEAR);

    updateHistogram();
}

void KisHistogramView::setView(double from, double size)
{
    m_from = from;
    m_width = size;
    if (m_from + m_width > 1.0)
        m_from = 1.0 - m_width;
    m_histogram -> producer() -> setView(m_from, m_width);

    m_histogram -> updateHistogram();
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

KisIDList KisHistogramView::listProducers()
{
    return KisHistogramProducerFactoryRegistry::instance() -> listKeysCompatibleWith(m_cs);
}

void KisHistogramView::setCurrentChannels(const KisID& producerID, vKisChannelInfoSP channels)
{
    m_currentProducer = KisHistogramProducerFactoryRegistry::instance()
            -> get(producerID) -> generate();
    m_currentProducer -> setView(m_from, m_width);
    m_histogram -> setProducer(m_currentProducer);
    m_histogram -> updateHistogram();
    m_histogram -> setChannel(0); // Set a default channel, just being nice

    m_channels.clear();
    m_channelToOffset.clear();

    if (channels.count() == 0) {
        updateHistogram();
        return;
    }

    vKisChannelInfoSP producerChannels = m_currentProducer -> channels();

    for (uint i = 0; i < channels.count(); i++) {
        // Also makes sure the channel is actually in the producer's list
        for (uint j = 0; j < producerChannels.count(); j++) {
            if (channels.at(i) -> name() == producerChannels.at(j) -> name()) {
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
        m_currentProducer -> setView(m_from, m_width);
        m_histogram -> setProducer(m_currentProducer);
        m_histogram -> updateHistogram();
    }

    m_channels.clear();
    m_channelToOffset.clear();

    if (!m_currentProducer) {
        updateHistogram();
        return;
    }

    if (info.isProducer) {
        m_color = true;
        m_channels = m_currentProducer -> channels();
        for (uint i = 0; i < m_channels.count(); i++)
            m_channelToOffset.append(i);
        m_histogram -> setChannel(0); // Set a default channel, just being nice
    } else {
        m_color = false;
        vKisChannelInfoSP channels = m_currentProducer -> channels();
        for (uint i = 0; i < channels.count(); i++) {
            KisChannelInfo* channel = channels.at(i);
            if (channel -> name() == info.channel -> name()) {
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
    m_histogram -> setHistogramType(type);
    updateHistogram();
}

void KisHistogramView::setChannels()
{
    m_comboInfo.clear();
    m_channelStrings.clear();
    m_channels.clear();
    m_channelToOffset.clear();

    KisIDList list = KisHistogramProducerFactoryRegistry::instance()
            -> listKeysCompatibleWith(m_cs);

    if (list.count() == 0) {
        kdDebug() << "Attention! No native histogram for this colorspace. Using converted RGB"
                  << endl;
        KisGenericRGBHistogramProducerFactory f;
        addProducerChannels(f.generate());
    } else {
        for (uint i = 0; i < list.count(); i++) {
            KisID id(*(list.at(i)));
            addProducerChannels( KisHistogramProducerFactoryRegistry::instance()
                    -> get(id) -> generate() );
        }
    }

    m_currentProducer = m_comboInfo.at(0).producer;
    m_color = false;
    m_channels.append(m_comboInfo.at(1).channel);
    m_channelToOffset.append(0);
}

void KisHistogramView::addProducerChannels(KisHistogramProducerSP producer) {
        ComboboxInfo info;
        info.isProducer = true;
        info.producer = producer;
        // channel not used for a producer
        vKisChannelInfoSP channels = info.producer -> channels();
        int count = channels.count();
        m_comboInfo.append(info);
        m_channelStrings.append(producer -> id() . name());
        for (int j = 0; j < count; j++) {
            info.isProducer = false;
            info.channel = channels.at(j);
            m_comboInfo.append(info);
            m_channelStrings.append(QString(" ").append(info.channel -> name()));
        }
}

void KisHistogramView::updateHistogram() 
{
    Q_UINT32 height = this -> height();
    
    if (!m_currentProducer) { // Something's very wrong
        kdDebug() << "No producer for this colorspace to update histogram with!!" << endl;
        return;
    }

    Q_INT32 bins = m_histogram -> producer() -> numberOfBins();
    m_pix = QPixmap(bins, height);
    m_pix.fill();
    QPainter p(&m_pix);
    p.setBrush(Qt::black);

    Q_INT32 i = 0;
    double highest = 0;

    // XXX ###
    // First we iterate once, so that we have the overall maximum. This is a bit inefficient,
    // so this needs to be done better. Preferably we let the KisHistogram do all calculations
    // once, and then just pick the right cached calculation each time (and extremum)
    for (uint chan = 0; chan < m_channels.count(); chan++) {
        m_histogram -> setChannel(m_channelToOffset.at(chan));
        m_histogram -> computeHistogram(); // We changed the channel, recompute
        if ((double)m_histogram -> getHighest() > highest)
            highest = (double)m_histogram -> getHighest();
    }

    for (uint chan = 0; chan < m_channels.count(); chan++) {
        m_histogram -> setChannel(m_channelToOffset.at(chan));
        m_histogram -> computeHistogram(); // We changed the channel, recompute

        if(m_color)
            p.setPen(m_channels.at(chan) -> color());

        if (m_histogram -> getHistogramType() == LINEAR) {
            double factor = (double)height / highest;
            for( i=0; i<bins; ++i ) {
                p.drawLine(i, height, i, height - int(m_histogram->getValue(i) * factor));
            }
        } else {
            double factor = (double)height / (double)log(highest);
            for( i = 0; i < bins; ++i ) {
                p.drawLine(i, height, i,
                           height - int(log((double)m_histogram->getValue(i)) * factor));
            }
        }
    }

    setPixmap(m_pix);
}

#include "kis_histogram_view.moc"
