/*
 *  SPDX-FileCopyrightText: 2020 Agata Cacko <tamtamy.tymona@gmail.com>
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "HistogramComputationStrokeStrategy.h"

#include "KoColorSpace.h"

#include "krita_utils.h"
#include "kis_image.h"
#include "kis_sequential_iterator.h"

struct HistogramComputationStrokeStrategy::Private
{

    class ProcessData : public KisStrokeJobData
    {
    public:
        ProcessData(QRect rect, int _jobId)
            : KisStrokeJobData(CONCURRENT)
            , rectToCalculate(rect)
            , jobId(_jobId)
        {}

        QRect rectToCalculate;
        int jobId; // id in the list of results
    };

    KisImageSP image;
    std::vector<HistVector> results;
    std::vector<HistVector> resultsLog;
    float maximumValue = 0.0f;
};


HistogramComputationStrokeStrategy::HistogramComputationStrokeStrategy(KisImageSP image)
    : KisIdleTaskStrokeStrategy(QLatin1String("ComputeHistogram"), kundo2_i18n("Update histogram"))
    , m_d(new Private)
{
    m_d->image = image;
}

HistogramComputationStrokeStrategy::~HistogramComputationStrokeStrategy()
{
}

void HistogramComputationStrokeStrategy::initStrokeCallback()
{
    KisIdleTaskStrokeStrategy::initStrokeCallback();

    QVector<KisStrokeJobData*> jobsData;
    int i = 0;
    QVector<QRect> tileRects = KritaUtils::splitRectIntoPatches(m_d->image->bounds(), KritaUtils::optimalPatchSize());
    m_d->results.resize(tileRects.size());
    m_d->resultsLog.resize(tileRects.size());

    Q_FOREACH (const QRect &tileRectangle, tileRects) {
        jobsData << new HistogramComputationStrokeStrategy::Private::ProcessData(tileRectangle, i);
        i++;
    }
    addMutatedJobs(jobsData);
}

void HistogramComputationStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    Private::ProcessData *d_pd = dynamic_cast<Private::ProcessData*>(data);

    if (!d_pd) {
        KisIdleTaskStrokeStrategy::doStrokeCallback(data);
        return;
    }

    QRect calculate = d_pd->rectToCalculate;

    KisPaintDeviceSP m_dev = m_d->image->projection();
    QRect imageBounds = m_d->image->bounds();

    const KoColorSpace *cs = m_dev->colorSpace();
    quint32 channelCount = m_dev->channelCount();
    quint32 pixelSize = m_dev->pixelSize();

    int imageSize = imageBounds.width() * imageBounds.height();
    int nSkip = 1 + (imageSize >> 20); //for speed use about 1M pixels for computing histograms

    if (calculate.isEmpty())
        return;

    initiateVector(m_d->results[d_pd->jobId], cs);
    initiateVector(m_d->resultsLog[d_pd->jobId], cs);

    quint32 toSkip = nSkip;

    float maximum = 1.0;
    QVector<float> channelValues(channelCount);
    if (cs->hasHighDynamicRange()) {
        KisSequentialConstIterator it(m_dev, calculate);

        int numConseqPixels = it.nConseqPixels();
        while (it.nextPixels(numConseqPixels)) {
            numConseqPixels = it.nConseqPixels();
            const quint8* pixel = it.rawDataConst();
            for (int k = 0; k < numConseqPixels; ++k) {
                cs->normalisedChannelsValue(pixel, channelValues);
                for (int chan = 0; chan < (int)channelCount; ++chan) {
                    maximum = qMax(maximum, channelValues.at(chan));
                }
                pixel += pixelSize;
            }
        }
    }
    m_d->maximumValue = maximum;

    const double maximumMultiplier = 255.0/maximum;
    const double logMaxMultiplier = 255.0/(std::log10(maximum*10)*0.1);


    KisSequentialConstIterator it(m_dev, calculate);

    int numConseqPixels = it.nConseqPixels();
    while (it.nextPixels(numConseqPixels)) {

        numConseqPixels = it.nConseqPixels();
        const quint8* pixel = it.rawDataConst();
        for (int k = 0; k < numConseqPixels; ++k) {
            cs->normalisedChannelsValue(pixel, channelValues);
            if (--toSkip == 0) {

                for (int chan = 0; chan < (int)channelCount; ++chan) {
                    m_d->results[d_pd->jobId][chan][ qBound(0, qRound(channelValues.at(chan)*maximumMultiplier), 255) ]++;
                    m_d->resultsLog[d_pd->jobId][chan][ qBound(0, qRound(std::log10(channelValues.at(chan)*10)*0.1*logMaxMultiplier), 255) ]++;
                }
                toSkip = nSkip;
            }
            pixel += pixelSize;
        }
    }
}

void HistogramComputationStrokeStrategy::finishStrokeCallback()
{
    HistogramData hisData;
    hisData.colorSpace = m_d->image->projection()->colorSpace();
    hisData.maximumValue = m_d->maximumValue;

    if (m_d->results.size() == 1) {
        hisData.bins = m_d->results[0];
        hisData.binsLog = m_d->resultsLog[0];
        Q_EMIT computationResultReady(hisData);
    } else {

        quint32 channelCount = m_d->image->projection()->channelCount();

        initiateVector(hisData.bins, hisData.colorSpace);
        initiateVector(hisData.binsLog, hisData.colorSpace);

        for (int chan = 0; chan < (int)channelCount; chan++) {
            int bsize = hisData.bins[chan].size();

            for (int bi = 0; bi < bsize; bi++) {
                hisData.bins[chan][bi] = 0;
                for (int i = 0; i < (int)m_d->results.size(); i++) {
                    hisData.bins[chan][bi] += m_d->results[i][chan][bi];
                }
            }
            int b2size = hisData.binsLog[chan].size();

            for (int bi = 0; bi < b2size; bi++) {
                hisData.binsLog[chan][bi] = 0;
                for (int i = 0; i < (int)m_d->resultsLog.size(); i++) {
                    hisData.binsLog[chan][bi] += m_d->resultsLog[i][chan][bi];
                }
            }
        }

        Q_EMIT computationResultReady(hisData);
    }

    KisIdleTaskStrokeStrategy::finishStrokeCallback();
}

void HistogramComputationStrokeStrategy::initiateVector(HistVector &vec, const KoColorSpace *colorSpace)
{
    vec.resize(colorSpace->channelCount());
    for (auto &bin : vec) {
        bin.resize(std::numeric_limits<quint8>::max() + 1);
    }
}
