/*
 *  SPDX-FileCopyrightText: 2020 Agata Cacko <tamtamy.tymona@gmail.com>
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef HISTOGRAMCOMPUTATIONSTROKESTRATEGY_H
#define HISTOGRAMCOMPUTATIONSTROKESTRATEGY_H

#include <KisIdleTaskStrokeStrategy.h>
#include <vector>

class KoColorSpace;


using HistVector = std::vector<std::vector<quint32> >; //Don't use QVector here - it's too slow for this purpose

struct HistogramData
{
    HistVector bins;
    const KoColorSpace* colorSpace {0};
};
Q_DECLARE_METATYPE(HistogramData)


class HistogramComputationStrokeStrategy : public KisIdleTaskStrokeStrategy
{
    Q_OBJECT
public:
    HistogramComputationStrokeStrategy(KisImageSP image);
    ~HistogramComputationStrokeStrategy() override;

private:
    void initStrokeCallback() override;
    void doStrokeCallback(KisStrokeJobData *data) override;
    void finishStrokeCallback() override;

    void initiateVector(HistVector &vec, const KoColorSpace* colorSpace);

Q_SIGNALS:
    //Emitted when thumbnail is updated and overviewImage is fully generated.
    void computationResultReady(HistogramData data);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // HISTOGRAMCOMPUTATIONSTROKESTRATEGY_H
