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
#include "HistogramComputationStrokeStrategy.h"
#include "KisWidgetWithIdleTask.h"

class KoColorSpace;

class HistogramDockerWidget : public KisWidgetWithIdleTask<QLabel>
{
    Q_OBJECT

public:
    HistogramDockerWidget(QWidget *parent = 0, const char *name = 0, Qt::WindowFlags f = Qt::WindowFlags());
    ~HistogramDockerWidget() override;
    void paintEvent(QPaintEvent *event) override;

public Q_SLOTS:
    void receiveNewHistogram(HistogramData data);
    void enableLogarithmic(bool enable);

private:
    KisIdleTasksManager::TaskGuard registerIdleTask(KisCanvas2 *canvas) override;
    void clearCachedState() override;

private:
    HistVector m_histogramData;
    HistVector m_histogramDataLog;
    const KoColorSpace* m_colorSpace {0};
    bool m_smoothHistogram {false};
    bool m_logarithmic{false};
    float m_maximumValue {1.0};
};

#endif // HISTOGRAMDOCKERWIDGET_H
