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
#include <kis_simple_stroke_strategy.h>

class KisCanvas2;
class KoColorSpace;


using HistVector = std::vector<std::vector<quint32> >; //Don't use QVector here - it's too slow for this purpose

struct HistogramData
{
    HistogramData() {}
    ~HistogramData() {}

    HistVector bins;
    const KoColorSpace* colorSpace;
};
Q_DECLARE_METATYPE(HistogramData)


class HistogramComputationStrokeStrategy : public QObject, public KisSimpleStrokeStrategy
{
    Q_OBJECT
public:
    HistogramComputationStrokeStrategy(KisImageWSP image);
    ~HistogramComputationStrokeStrategy() override;


private:
    void initStrokeCallback() override;
    void doStrokeCallback(KisStrokeJobData *data) override;
    void finishStrokeCallback() override;
    void cancelStrokeCallback() override;

    void initiateVector(HistVector &vec, const KoColorSpace* colorSpace);

Q_SIGNALS:
    //Emitted when thumbnail is updated and overviewImage is fully generated.
    void computationResultReady(HistogramData data);


private:
    struct Private;
    const QScopedPointer<Private> m_d;
    KisImageSP m_image;
    std::vector<HistVector> m_results;
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
    void receiveNewHistogram(HistogramData data);


private:
    HistVector m_histogramData;
    const KoColorSpace* m_colorSpace;
    bool m_smoothHistogram;
};

#endif // HISTOGRAMDOCKERWIDGET_H
