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
    void setPaintDevice(KisCanvas2* canvas);
    void paintEvent(QPaintEvent *event) override;

public Q_SLOTS:
    void updateHistogram();
    void receiveNewHistogram(HistVector*);

private:
    KisPaintDeviceSP m_paintDevice;
    HistVector m_histogramData;
    QRect m_bounds;
    bool m_smoothHistogram;
};

#endif // HISTOGRAMDOCKERWIDGET_H
