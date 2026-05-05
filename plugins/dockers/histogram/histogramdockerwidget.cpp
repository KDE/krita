/*
 *  SPDX-FileCopyrightText: 2016 Eugene Ingerman <geneing at gmail dot com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "histogramdockerwidget.h"

#include <QThread>
#include <limits>
#include <algorithm>
#include <QTime>
#include <QPainter>
#include <QPainterPath>
#include <functional>

#include "KoChannelInfo.h"
#include "KisViewManager.h"
#include "kis_canvas2.h"



HistogramDockerWidget::HistogramDockerWidget(QWidget *parent, const char *name, Qt::WindowFlags f)
    : KisWidgetWithIdleTask<QLabel>(parent, f)
{
    setObjectName(name);
    qRegisterMetaType<HistogramData>();
}

HistogramDockerWidget::~HistogramDockerWidget()
{
}

void HistogramDockerWidget::receiveNewHistogram(HistogramData data)
{
    m_histogramData = data.bins;
    m_histogramDataLog = data.binsLog;
    m_colorSpace = data.colorSpace;
    m_maximumValue = data.maximumValue;
    update();
}

void HistogramDockerWidget::enableLogarithmic(bool enable)
{
    if (m_logarithmic == enable) return;
    m_logarithmic = enable;
    update();
}

KisIdleTasksManager::TaskGuard HistogramDockerWidget::registerIdleTask(KisCanvas2 *canvas)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(canvas, KisIdleTasksManager::TaskGuard());

    return
        canvas->viewManager()->idleTasksManager()->
        addIdleTaskWithGuard([this](KisImageSP image) {
            HistogramComputationStrokeStrategy* strategy =
                new HistogramComputationStrokeStrategy(image);

            connect(strategy, SIGNAL(computationResultReady(HistogramData)), this, SLOT(receiveNewHistogram(HistogramData)));

            return strategy;
        });
}

void HistogramDockerWidget::clearCachedState()
{
    m_colorSpace = 0;
    m_histogramData.clear();
    m_histogramDataLog.clear();
}

QMap<float, float> calculateLogGridLines(int width, qreal maximumValue) {
    int power = 1;
    QList<float> mainLines = {0.0, 0.25, 0.5, 1.0};
    while (mainLines.last() < (maximumValue)) {
        qreal mainLine = powf(10, power);
        mainLines.append(mainLine * 0.25);
        mainLines.append(mainLine * 0.5);
        mainLines.append(mainLine * 0.75);
        mainLines.append(mainLine);

        power += 1;
    }
    while (mainLines.last() > maximumValue) {
        mainLines.removeLast();
    }
     mainLines.append(maximumValue);

    QMap<float, float> gridLines;
    Q_FOREACH(float mainLine, mainLines) {
        if (qFuzzyCompare(mainLine, 0.f)) {
            gridLines.insert(0.0, 0.0);
        } else {
            gridLines.insert(mainLine, std::log(mainLine+1)*(width/std::log(mainLines.last()+1)));
        }
    }

    return gridLines;
}

QMap<float, float> calculateLinearGridLines(int width, qreal maximumValue)
{
    float gridValue = 0.25;
    float gridWidthLength = width / (maximumValue / gridValue);
    while(gridWidthLength < 25) {
        gridValue *= 2;
        gridWidthLength = width / (maximumValue / gridValue);
    }
    QMap<float, float> gridLines;
    for (float i = 0; i < float(width); i+=gridWidthLength) {
        gridLines.insert((i/gridWidthLength)*gridValue, i);
    }
    return gridLines;
}

void HistogramDockerWidget::paintEvent(QPaintEvent *event)
{
    if (m_colorSpace && !m_histogramData.empty()) {
        int nBins = m_logarithmic? m_histogramData.at(0).size(): m_histogramDataLog.at(0).size();
        const KoColorSpace* cs = m_colorSpace;

        QLabel::paintEvent(event);
        QPainter painter(this);
        painter.fillRect(0, 0, this->width(), this->height(), this->palette().dark().color());
        painter.setPen(this->palette().light().color());

        const int gridHeight = this->height() - painter.fontMetrics().height();

        painter.save();

        QMap<float, float> horGrid = m_logarithmic? calculateLogGridLines(this->width(), m_maximumValue): calculateLinearGridLines(this->width(), m_maximumValue);
        Q_FOREACH (float k, horGrid.keys()) {
            float i = horGrid.value(k);
            painter.drawLine(qRound(i), 0, qRound(i), gridHeight);
            painter.drawText(QPointF(qRound(i), this->height()-2), QString::number(k));
        }

        unsigned int nChannels = cs->channelCount();
        const QList<KoChannelInfo *> channels = cs->channels();
        unsigned int highest = 0;
        //find the most populous bin in the histogram to scale it properly
        for (int chan = 0; chan < channels.size(); chan++) {
            if (channels.at(chan)->channelType() != KoChannelInfo::ALPHA) {
                std::vector<quint32> histogramTemp = m_logarithmic? m_histogramDataLog.at(chan): m_histogramData.at(chan);
                //use 98th percentile, rather than max for better visual appearance
                int nthPercentile = 2 * histogramTemp.size() / 100;
                //unsigned int max = *std::max_element(m_histogramData.at(chan).begin(),m_histogramData.at(chan).end());
                std::nth_element(histogramTemp.begin(),
                                 histogramTemp.begin() + nthPercentile, histogramTemp.end(), std::greater<int>());
                unsigned int max = *(histogramTemp.begin() + nthPercentile);

                highest = std::max(max, highest);
            }
        }

        QMap<float, float> vertGrid = m_logarithmic? calculateLogGridLines(gridHeight, highest): calculateLinearGridLines(gridHeight, highest);
        Q_FOREACH (float k, vertGrid.keys()) {
            float i = vertGrid.value(k);
            painter.drawLine(0., gridHeight -i, this->width(), gridHeight -i);
        }

        painter.restore();

        painter.setCompositionMode(QPainter::CompositionMode_Plus);
        const float barWidth = float(this->width()-1) / float(nBins);
        float logVerticalMax = std::log(vertGrid.keys().last()+1);

        for (int chan = 0; chan < (int)nChannels; chan++) {
            if (channels.at(chan)->channelType() != KoChannelInfo::ALPHA) {
                QColor color = channels.at(chan)->color();

                //special handling of grayscale color spaces. can't use color returned above.
                if(cs->colorChannelCount()==1){
                    color = QColor(Qt::gray);
                }

                QColor fill_color = color;
                fill_color.setAlphaF(.25);
                painter.setBrush(fill_color);
                QPen pen = QPen(color);
                pen.setWidth(0);
                painter.setPen(pen);

                if (m_smoothHistogram) {
                    QPainterPath path;
                    path.moveTo(QPointF(1, gridHeight));
                    for (qint32 i = 0; i < nBins; ++i) {
                        const float curBinVertical = !m_logarithmic? m_histogramData[chan][i]
                                                                    : std::log(m_histogramDataLog[chan][i]+1);
                        const float maxBin = !m_logarithmic? highest: logVerticalMax;
                        const float v = curBinVertical > 0? ((std::max((float)maxBin - curBinVertical, 0.f))/maxBin)*gridHeight: 0;
                        path.lineTo(QPointF((i*barWidth) - (barWidth*0.5), v));

                    }
                    path.lineTo(QPointF(this->width(), gridHeight));
                    path.closeSubpath();
                    painter.drawPath(path);
                } else {
                    painter.setBrush(color);
                    painter.setPen(Qt::transparent);
                    float start = 1.0;
                    for (qint32 i = 0; i < nBins; ++i) {

                        const float curBinEnd = 1.0+(i*barWidth)+barWidth;
                        const float curBinVertical = m_logarithmic? std::log(m_histogramDataLog[chan][i] +1): m_histogramData[chan][i];
                        const float maxBin = m_logarithmic? logVerticalMax: highest;
                        if (curBinVertical > 0) {
                            const int v = std::max(qRound((curBinVertical/maxBin)*gridHeight), 1);
                            const QRect bar(QPoint(qRound(start), std::max(gridHeight - v, 0)),
                                            QPoint(qRound(curBinEnd), gridHeight));
                            painter.drawRect(bar);
                        }
                        start = curBinEnd;
                    }
                }
            }
        }
    }
}
