/*
 *  Copyright (c) 2016 Eugene Ingerman <geneing at gmail dot com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
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


#ifndef HISTOGRAMDOCKERWIDGET_H
#define HISTOGRAMDOCKERWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QThread>
#include "kis_types.h"
#include <vector>

class KisCanvas2;
class KoColorSpace;

typedef std::vector<std::vector<quint32> > HistVector; //Don't use QVector here - it's too slow for this purpose


class HistogramComputationThread : public QThread
{
    Q_OBJECT
public:
    HistogramComputationThread(KisPaintDeviceSP _dev, const QRect& _bounds) : m_dev(_dev), m_bounds(_bounds)
    {}

    void run() override;

Q_SIGNALS:
    void resultReady(HistVector*);

private:
    KisPaintDeviceSP m_dev;
    QRect m_bounds;
    HistVector bins;
};


class HistogramDockerWidget : public QLabel
{
    Q_OBJECT

public:
    HistogramDockerWidget(QWidget *parent = 0, const char *name = 0, Qt::WindowFlags f = 0);
    ~HistogramDockerWidget() override;
    void paintEvent(QPaintEvent *event) override;

public Q_SLOTS:
    /**
     * @brief updateHistogram starts calculation of the histogram
     * @param canvas canvas that the calculations must be based on
     *
     * Note: don't try to save the paint device of the projection of the image.
     * Paint device of the projection changes in multiple cases, for example
     * Isolate Mode or when opening an image with a single layer.
     */
    void updateHistogram(KisCanvas2* canvas);
    void receiveNewHistogram(HistVector*);

private:
    HistVector m_histogramData;
    const KoColorSpace* m_colorSpace;
    bool m_smoothHistogram;
};

#endif // HISTOGRAMDOCKERWIDGET_H
