/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisRollingMeanAccumulatorWrapper.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>

using namespace boost::accumulators;

struct KisRollingMeanAccumulatorWrapper::Private {
    Private(int windowSize)
        : accumulator(tag::rolling_window::window_size = windowSize)
    {
    }

    accumulator_set<qreal, stats<tag::lazy_rolling_mean> > accumulator;
};


KisRollingMeanAccumulatorWrapper::KisRollingMeanAccumulatorWrapper(int windowSize)
    : m_d(new Private(windowSize))
{
}

KisRollingMeanAccumulatorWrapper::~KisRollingMeanAccumulatorWrapper()
{
}

void KisRollingMeanAccumulatorWrapper::operator()(qreal value)
{
    m_d->accumulator(value);
}

qreal KisRollingMeanAccumulatorWrapper::rollingMean() const
{
    return boost::accumulators::rolling_mean(m_d->accumulator);
}

qreal KisRollingMeanAccumulatorWrapper::rollingMeanSafe() const
{
    return boost::accumulators::rolling_count(m_d->accumulator) > 0 ?
        boost::accumulators::rolling_mean(m_d->accumulator) : 0;
}

int KisRollingMeanAccumulatorWrapper::rollingCount() const
{
    return boost::accumulators::rolling_count(m_d->accumulator);
}

void KisRollingMeanAccumulatorWrapper::reset(int windowSize)
{
    m_d->accumulator =
        accumulator_set<qreal, stats<tag::lazy_rolling_mean>>(
            tag::rolling_window::window_size = windowSize);
}
