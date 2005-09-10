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
#include "kis_histogram_widget.h"
#include "kis_histogram.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_abstract_colorspace.h"

KisHistogramWidget::KisHistogramWidget(QWidget *parent, const char *name) 
    : super(parent, name)
{
}

KisHistogramWidget::~KisHistogramWidget()
{
}

void KisHistogramWidget::setChannels()
{
    m_comboInfo.clear();
    KisIDList list = KisHistogramProducerFactoryRegistry::instance()
            -> listKeysCompatibleWith(m_cs);

    for (uint i = 0; i < list.count(); i++) {
        ComboboxInfo info;
        info.isProducer = true;
        KisID id(*(list.at(i)));
        info.producer = KisHistogramProducerFactoryRegistry::instance()
                -> get(id) -> generate();
        // channel not used for a producer
        vKisChannelInfoSP channels = info.producer -> channels();
        int count = channels.count();
        m_comboInfo.append(info);
        cmbChannel -> insertItem(id . name());
        for (int j = 0; j < count; j++) {
            info.isProducer = false;
            info.channel = channels.at(j);
            m_comboInfo.append(info);
            cmbChannel -> insertItem(QString(" ").append(info.channel -> name()));
        }
    }

    m_currentProducer = m_comboInfo.at(0).producer;
    cmbChannel -> setCurrentItem(1);
    m_color = false;
    m_channels.clear();
    m_channels.append(m_comboInfo.at(1).channel);
    m_channelToOffset.clear();
    m_channelToOffset.append(0);
}

void KisHistogramWidget::setLayer(KisLayerSP layer) 
{
    grpType -> disconnect(this);
    cmbChannel -> disconnect(this);

    m_cs = layer -> colorSpace();

    setChannels(); // Sets m_currentProducer to the first in the list
    kdDebug() << "Histogram using the Histogram Producer "
            << m_currentProducer -> id().name() << endl;

    // View display
    currentView -> setMinValue(0);
    currentView -> setMaxValue(100);
    if (m_currentProducer -> maximalZoom() < 1.0) {
        currentView -> setEnabled(true);
    } else {
        currentView -> setEnabled(false);
    }
    m_from = m_currentProducer -> viewFrom();
    m_width = m_currentProducer -> viewWidth();

    m_histogram = new KisHistogram(layer, m_currentProducer, LINEAR);

    updateHistogram();

    connect(grpType, SIGNAL(clicked(int)), this, SLOT(slotTypeSwitched(int)));
    connect(cmbChannel, SIGNAL(activated(int)), this, SLOT(setActiveChannel(int)));
    connect(zoomIn, SIGNAL(clicked()), this, SLOT(slotZoomIn()));
    connect(zoomOut, SIGNAL(clicked()), this, SLOT(slotZoomOut()));
    connect(currentView, SIGNAL(valueChanged(int)), this, SLOT(slide(int)));
}

void KisHistogramWidget::setActiveChannel(int channel)
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

void KisHistogramWidget::slotTypeSwitched(int id)
{
    if (id == LINEAR)
        m_histogram -> setHistogramType(LINEAR);
    else if (id == LOGARITHMIC)
        m_histogram -> setHistogramType(LOGARITHMIC);

    updateHistogram();
}

void KisHistogramWidget::setView(double from, double size)
{
    m_from = from;
    m_width = size;
    if (m_from + m_width > 1.0)
        m_from = 1.0 - m_width;
    m_histogram -> producer() -> setView(m_from, m_width);

    m_histogram -> updateHistogram();
    updateHistogram();

    if (m_currentProducer -> maximalZoom() < 1.0) {
        if ((m_width / 2) >= m_currentProducer -> maximalZoom()) {
            zoomIn -> setEnabled(true);
        } else {
            zoomIn -> setEnabled(false);
        }
        if (m_width * 2 <= 1.0) {
            zoomOut -> setEnabled(true);
        } else {
            zoomOut -> setEnabled(false);
        }
        if (m_width < 1.0)
            currentView -> setEnabled(true);
        else
            currentView -> setEnabled(false);
    }
}

void KisHistogramWidget::updateHistogram() 
{
    Q_UINT32 height = pixHistogram -> height();
    Q_INT32 bins = m_histogram -> producer() -> numberOfBins();
    m_pix = QPixmap(bins, height);
    m_pix.fill();
    QPainter p(&m_pix);
    p.setBrush(Qt::black);
    
    Q_INT32 i = 0;
    
    for (uint chan = 0; chan < m_channels.count(); chan++) {
        m_histogram -> setChannel(m_channelToOffset.at(chan));
        m_histogram -> computeHistogram(); // We changed the channel, recompute

        if(m_color)
            p.setPen(m_channels.at(chan) -> color());

        if (m_histogram -> getHistogramType() == LINEAR) {
            double factor = (double)height / (double)m_histogram -> getHighest();
            for( i=0; i<bins; ++i ) {
                p.drawLine(i, height, i, height - int(m_histogram->getValue(i) * factor));
            }
        } else {
            double factor = (double)height / (double)log(m_histogram -> getHighest());
            for( i = 0; i < bins; ++i ) {
                p.drawLine(i, height, i, height - int(log((double)m_histogram->getValue(i)) * factor));
            }
        }
    }

    pixHistogram -> setPixmap(m_pix);
}

void KisHistogramWidget::slotZoomIn() {
    if ((m_width / 2) >= m_currentProducer -> maximalZoom()) {
        setView(m_currentProducer -> viewFrom(), m_width / 2);
    }
}

void KisHistogramWidget::slotZoomOut() {
    if (m_width * 2 <= 1.0) {
        setView(m_currentProducer -> viewFrom(), m_width * 2);
    }
}

void KisHistogramWidget::slide(int val) {
    kdDebug() << "slided to " << val << " viewing from: "
            << ((static_cast<double>(val) / 100.0) * (1.0 - m_width)) << endl;
    // Beware: at the END (e.g. 100), we want to still view m_width:
    setView((static_cast<double>(val) / 100.0) * (1.0 - m_width), m_width);
}

#include "kis_histogram_widget.moc"

