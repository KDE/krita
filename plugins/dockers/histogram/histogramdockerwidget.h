/*
 *  SPDX-FileCopyrightText: 2016 Eugene Ingerman <geneing at gmail dot com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
    HistogramDockerWidget(QWidget *parent = 0, const char *name = 0, Qt::WindowFlags f = Qt::WindowFlags());
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
