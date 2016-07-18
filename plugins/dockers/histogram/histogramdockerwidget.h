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


#ifndef HISTOGRAMDOCKERWIDGET_H
#define HISTOGRAMDOCKERWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QThread>
#include "kis_types.h"

typedef QVector<QVector<quint32> > HistVector;


class HistogramComputationThread : public QThread
{
    Q_OBJECT
public:
    HistogramComputationThread(KisPaintDeviceSP _dev) : m_dev(_dev)
    {}

    void run() Q_DECL_OVERRIDE;

Q_SIGNALS:
    void resultReady(HistVector*);

private:
    KisPaintDeviceSP m_dev;
    HistVector bins;
};


class HistogramDockerWidget : public QLabel
{
    Q_OBJECT

public:
    HistogramDockerWidget(QWidget *parent = 0, const char *name = 0, Qt::WindowFlags f = 0);
    ~HistogramDockerWidget();
    void setPaintDevice( KisPaintDeviceSP dev );
    void paintEvent(QPaintEvent *event);

public Q_SLOTS:
    void updateHistogram();
    void receiveNewHistogram(HistVector*);

private:
    KisPaintDeviceSP m_paintDevice;
    HistVector m_histogramData;

};

#endif // HISTOGRAMDOCKERWIDGET_H
