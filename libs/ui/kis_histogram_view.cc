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

#include "kis_histogram_view.h"

#include <math.h>

#include <QPainter>
#include <QPixmap>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QScrollBar>
#include <QMouseEvent>
#include <QFrame>
#include <QBrush>
#include <QLinearGradient>
#include <QImage>
#include <QPaintEvent>

#include <kis_debug.h>

#include "KoChannelInfo.h"
#include "KoBasicHistogramProducers.h"
#include "KoColorSpace.h"

#include "kis_global.h"
#include "kis_layer.h"
#include <kis_signal_compressor.h>
#include "kis_paint_device.h"

KisHistogramView::KisHistogramView(QWidget *parent, const char *name, Qt::WindowFlags f)
        : QLabel(parent, f),
          m_currentDev(nullptr), m_currentProducer(nullptr),
          m_smoothHistogram(false),m_histogram_type(LINEAR)
{
    setObjectName(name);
}

KisHistogramView::~KisHistogramView()
{
}


KoHistogramProducer *KisHistogramView::currentProducer()
{
    return m_currentProducer;
}

void KisHistogramView::startUpdateCanvasProjection()
{
    updateHistogramCalculation();
}

void KisHistogramView::setChannels(QList<KoChannelInfo*> & channels)
{
    m_channels = channels;
    updateHistogramCalculation();
}

void KisHistogramView::setProducer(KoHistogramProducer* producer)
{
    m_currentProducer = producer;
    m_channels = m_currentProducer->channels();
    if( !m_histogram.isNull() ){
        m_histogram->setProducer( m_currentProducer );
    }
    updateHistogramCalculation();
}

void KisHistogramView::setPaintDevice(KisPaintDeviceSP dev, KoHistogramProducer* producer, const QRect &bounds)
{
    m_currentProducer = producer;
    m_channels = m_currentProducer->channels();
    m_currentDev = dev;
    m_currentBounds = bounds;
    m_histogram = new KisHistogram(m_currentDev, m_currentBounds, m_currentProducer, m_histogram_type);

    updateHistogramCalculation();
}

void KisHistogramView::setView(double from, double size)
{
    double m_from = from;
    double m_width = size;
    if (m_from + m_width > 1.0)
        m_from = 1.0 - m_width;
    m_histogram->producer()->setView(m_from, m_width);
    updateHistogramCalculation();
}

bool KisHistogramView::hasColor()
{
    return m_color;
}

void KisHistogramView::setColor(bool set)
{
    if (set != m_color) {
        m_color = set;
    }
    update();
}

void KisHistogramView::setHistogramType(enumHistogramType type)
{
    m_histogram_type = type;
    updateHistogramCalculation();
}

void KisHistogramView::updateHistogramCalculation()
{
    if (!m_currentProducer || m_currentDev.isNull() || m_histogram.isNull() ) { // Something's very wrong: not initialized
        return;
    }
    m_histogram->updateHistogram();
    update();
}

void KisHistogramView::mousePressEvent(QMouseEvent * e)
{
    if (e->button() == Qt::RightButton)
        emit rightClicked(e->globalPos());
    else
        QLabel::mousePressEvent(e);
}

void KisHistogramView::setSmoothHistogram(bool smoothHistogram)
{
    m_smoothHistogram = smoothHistogram;
}


void KisHistogramView::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);

    if( this->height() > 0 && this->width()>0 && !m_histogram.isNull() ){
        qint32 height = this->height();
        int selFrom, selTo; // from - to in bins

        qint32 bins = m_histogram->producer()->numberOfBins();

        QPainter painter(this);
        painter.setPen(this->palette().light().color());

        const int NGRID = 4;
        for(int i=0; i<=NGRID; ++i){
            painter.drawLine(this->width()*i/NGRID,0.,this->width()*i/NGRID,this->height());
            painter.drawLine(0.,this->height()*i/NGRID,this->width(),this->height()*i/NGRID);
        }


        // Draw the box of the selection, if any
        if (m_histogram->hasSelection()) {
            double width = m_histogram->selectionTo() - m_histogram->selectionFrom();
            double factor = static_cast<double>(bins) / m_histogram->producer()->viewWidth();
            selFrom = static_cast<int>(m_histogram->selectionFrom() * factor);
            selTo = selFrom + static_cast<int>(width * factor);
            painter.drawRect(selFrom, 0, selTo - selFrom, height);
        } else {
            // We don't want the drawing to think we're in a selected area
            selFrom = -1;
            selTo = 2;
        }

        float highest = 0;

        if(m_channels.count()==0){
            m_channels=m_currentProducer->channels();
        }
        int nChannels = m_channels.size();

        // First we iterate once, so that we have the overall maximum. This is a bit inefficient,
        // but not too much since the histogram caches the calculations
        for (int chan = 0; chan < m_channels.size(); chan++) {
            if( m_channels.at(chan)->channelType() != KoChannelInfo::ALPHA ){
                m_histogram->setChannel(chan);
                if ((float)m_histogram->calculations().getHighest() > highest)
                    highest = (float)m_histogram->calculations().getHighest();
            }
        }
        highest = (m_histogram_type==LINEAR)? highest: std::log2(highest);

        painter.setWindow(QRect(-1,0,bins+1,highest));
        painter.setCompositionMode(QPainter::CompositionMode_Plus);

        for (int chan = 0; chan < nChannels; chan++) {
            if( m_channels.at(chan)->channelType() != KoChannelInfo::ALPHA ){
                auto color = m_channels.at(chan)->color();
                auto fill_color = color;
                fill_color.setAlphaF(.25);
                painter.setBrush(fill_color);
                auto pen = QPen(color);
                pen.setWidth(0);
                painter.setPen(pen);

                if (m_smoothHistogram){
                    QPainterPath path;

                    m_histogram->setChannel(chan);
                    path.moveTo(QPointF(-1,highest));
                    for (qint32 i = 0; i < bins; ++i) {
                        float v = (m_histogram_type==LINEAR)? highest-m_histogram->getValue(i): highest-std::log2(m_histogram->getValue(i));
                        path.lineTo(QPointF(i,v));

                    }
                    path.lineTo(QPointF(bins+1,highest));
                    path.closeSubpath();
                    painter.drawPath(path);
                }
                else {
                    pen.setWidth(1);
                    painter.setPen(pen);
                    m_histogram->setChannel(chan);
                    for (qint32 i = 0; i < bins; ++i) {
                        float v = (m_histogram_type==LINEAR)? highest-m_histogram->getValue(i): highest-std::log2(m_histogram->getValue(i));
                        painter.drawLine(QPointF(i,highest),QPointF(i,v));
                    }
                }
            }
        }
    }
}
