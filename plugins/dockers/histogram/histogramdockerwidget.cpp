/*
 *  Copyright (c) 2016 Eugene Ingerman <geneing at gmail dot com>
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

#include "histogramdockerwidget.h"

#include <QThread>
#include <QVector>
#include <limits>
#include <algorithm>
#include <QTime>
#include <QPainter>

#include "KoChannelInfo.h"
#include "kis_paint_device.h"
#include "KoColorSpace.h"
#include <kis_iterator_ng.h>


HistogramDockerWidget::HistogramDockerWidget(QWidget *parent, const char *name, Qt::WindowFlags f)
    : QLabel(parent, f), m_paintDevice(nullptr)
{
    setObjectName(name);
}

HistogramDockerWidget::~HistogramDockerWidget()
{

}

void HistogramDockerWidget::setPaintDevice(KisPaintDeviceSP dev)
{
    m_paintDevice = dev;
}

void HistogramDockerWidget::updateHistogram()
{
    QTime t;
    t.start();

    if (!m_paintDevice.isNull()) {
        KisPaintDeviceSP m_devClone = new KisPaintDevice(m_paintDevice->colorSpace());

        m_devClone->makeCloneFrom(m_paintDevice, m_paintDevice->exactBounds());

        HistogramComputationThread *workerThread = new HistogramComputationThread(m_devClone);
        connect(workerThread, &HistogramComputationThread::resultReady, this, &HistogramDockerWidget::receiveNewHistogram);
        connect(workerThread, &HistogramComputationThread::finished, workerThread, &QObject::deleteLater);
        workerThread->start();
    }
    qDebug() << QTime::currentTime() << "+  " << __FUNCTION__ << ":" << __LINE__ << " " << "updateHistogram" << " " << t.elapsed();
}

void HistogramDockerWidget::receiveNewHistogram(HistVector *histogramData)
{
    m_histogramData = *histogramData;
    update();
}

void HistogramDockerWidget::paintEvent(QPaintEvent *event)
{
    if( !m_histogramData.isEmpty() ){
        int nBins = m_histogramData.at(0).size();

        QLabel::paintEvent(event);
        QPainter painter(this);
        painter.setPen(this->palette().light().color());

        const int NGRID = 4;
        for(int i=0; i<=NGRID; ++i){
            painter.drawLine(this->width()*i/NGRID,0.,this->width()*i/NGRID,this->height());
            painter.drawLine(0.,this->height()*i/NGRID,this->width(),this->height()*i/NGRID);
        }

        unsigned int nChannels = m_paintDevice->colorSpace()->channelCount();
        QList<KoChannelInfo *> channels = m_paintDevice->colorSpace()->channels();
        unsigned int highest=0;
        //find the most populous bin in the histogram to scale it properly
        for (int chan = 0; chan < channels.size(); chan++) {
            if( channels.at(chan)->channelType() != KoChannelInfo::ALPHA ){
                unsigned int max = *std::max_element(m_histogramData.at(chan).constBegin(),m_histogramData.at(chan).constEnd());
                highest = std::max(max,highest);
            }
        }

        painter.setWindow(QRect(-1,0,nBins+1,highest));
        painter.setCompositionMode(QPainter::CompositionMode_Plus);

        for (int chan = 0; chan < nChannels; chan++) {
            if( channels.at(chan)->channelType() != KoChannelInfo::ALPHA ){
                QColor color = channels.at(chan)->color();
                QColor fill_color = color;
                fill_color.setAlphaF(.25);
                painter.setBrush(fill_color);
                QPen pen = QPen(color);
                pen.setWidth(0);
                painter.setPen(pen);

                if (0){
                    QPainterPath path;
                    path.moveTo(QPointF(-1,highest));
                    for (qint32 i = 0; i < nBins; ++i) {
                        float v = highest-m_histogramData.at(chan).at(i);
                        path.lineTo(QPointF(i,v));

                    }
                    path.lineTo(QPointF(nBins+1,highest));
                    path.closeSubpath();
                    painter.drawPath(path);
                }
                else {
                    pen.setWidth(1);
                    painter.setPen(pen);
                    for (qint32 i = 0; i < nBins; ++i) {
                        float v = highest-m_histogramData.at(chan).at(i);
                        painter.drawLine(QPointF(i,highest),QPointF(i,v));
                    }
                }
            }
        }
    }
}

void HistogramComputationThread::run()
{
    QTime t;
    t.start();

    const KoColorSpace *cs = m_dev->colorSpace();
    quint32 channelCount = m_dev->channelCount();

    //allocate space for the data
    bins.resize((int)channelCount);
    for (auto &bin : bins) {
        bin.resize(std::numeric_limits<quint8>::max()+1);
    }

    KisSequentialConstIterator it(m_dev, m_dev->exactBounds());
    do {
        const quint8* pixel = it.rawDataConst();
        for (int i = 0; i < (int)channelCount; ++i) {
            ++(bins[i][cs->scaleToU8(pixel, i)]);
        }
    } while (it.nextPixel());

    qDebug() << QTime::currentTime() << "+  " << __FUNCTION__ << ":" << __LINE__ << " " << "run" << " " << t.elapsed();

    emit resultReady(&bins);
}
