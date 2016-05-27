/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "histogramwidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QCursor>

#include <KoCanvasController.h>
#include <KoZoomController.h>
#include <KoChannelInfo.h>

#include <kis_canvas2.h>
#include <KisViewManager.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_signal_compressor.h>
#include <kis_config.h>

HistogramWidget::HistogramWidget(QWidget * parent)
    : QLabel(parent)
    , m_compressor(new KisSignalCompressor(500, KisSignalCompressor::POSTPONE, this))
    , m_canvas(0)
{
    this->setPalette(parent->palette());
    connect(m_compressor, SIGNAL(timeout()), SLOT(startUpdateHistogram()));
}

HistogramWidget::~HistogramWidget()
{
}

void HistogramWidget::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas) {
        m_canvas->image()->disconnect(this);
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

    if (m_canvas) {
        connect(m_canvas->image(), SIGNAL(sigImageUpdated(QRect)), m_compressor, SLOT(start()), Qt::UniqueConnection);
        m_compressor->start();

        auto cs = m_canvas->image()->colorSpace();
        QList<QString> producers = KoHistogramProducerFactoryRegistry::instance()->keysCompatibleWith(cs);
        KoHistogramProducer *producer = KoHistogramProducerFactoryRegistry::instance()->get(producers.at(0))->generate();
        m_histogram = new KisHistogram(m_canvas->image()->projection(),m_canvas->image()->bounds(),producer,LINEAR);
        m_histogram->updateHistogram();
        startUpdateHistogram();
    }
}


void HistogramWidget::startUpdateHistogram( void )
{
    if (!m_canvas) return;

    m_histogram->updateHistogram();
    qint32 bins = m_histogram->producer()->numberOfBins();

    QPixmap m_pix(this->size());
    m_pix.fill(this->palette().mid().color());

    QPainter painter(&m_pix);
    painter.setPen(this->palette().light().color());
    const int NGRID = 4;
    for(int i=0; i<=NGRID; ++i){
        painter.drawLine(m_pix.width()*i/NGRID,0.,m_pix.width()*i/NGRID,m_pix.height());
        painter.drawLine(0.,m_pix.height()*i/NGRID,m_pix.width(),m_pix.height()*i/NGRID);
    }

    double highest = 0;

    auto m_channels = m_histogram->producer()->channels();
    int nChannels = m_channels.size();

    // First we iterate once, so that we have the overall maximum. This is a bit inefficient,
    // but not too much since the histogram caches the calculations
    for (int chan = 0; chan < nChannels; chan++) {
        if( m_channels.at(chan)->channelType() != KoChannelInfo::ALPHA ){
            m_histogram->setChannel(chan);
            if ((double)m_histogram->calculations().getHighest() > highest)
                highest = (double)m_histogram->calculations().getHighest();
        }
    }

    QPixmap hist_pix(this->size());
    hist_pix.fill(QColor(0,0,0,0));

    painter.setWindow(QRect(-1,0,bins,highest+1));
    painter.setCompositionMode(QPainter::CompositionMode_Plus);

    for (int chan = 0; chan < nChannels; chan++) {
        if( m_channels.at(chan)->channelType() != KoChannelInfo::ALPHA ){
            QPainterPath path;

            m_histogram->setChannel(chan);
            path.moveTo(-1,highest);
            for (qint32 i = 0; i < bins; ++i) {
                path.lineTo(i,highest-int(m_histogram->getValue(i)));
            }
            path.lineTo(bins+1,highest);
            path.closeSubpath();

            auto color = m_channels.at(chan)->color();
            auto fill_color = color;
            fill_color.setAlphaF(.25);
            painter.setBrush(fill_color);
            auto pen = QPen(color);
            pen.setWidth(1);
            painter.setPen(pen);

            painter.drawPath(path);
        }
    }

    painter.drawPixmap(0,0,hist_pix);
    this->setPixmap(m_pix);
    update();
}

void HistogramWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    m_compressor->start();
}

void HistogramWidget::mousePressEvent(QMouseEvent* event)
{
    event->accept();
    update();
}

void HistogramWidget::mouseMoveEvent(QMouseEvent* event)
{
    event->accept();
}

void HistogramWidget::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept();
    update();
}


