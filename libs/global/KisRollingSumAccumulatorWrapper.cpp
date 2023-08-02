/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "KisRollingSumAccumulatorWrapper.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/rolling_sum.hpp>
#include <boost/accumulators/statistics/rolling_count.hpp>

using namespace boost::accumulators;

struct KisRollingSumAccumulatorWrapper::Private {
    Private(int windowSize)
        : accumulator(tag::rolling_window::window_size = windowSize)
    {
    }

    accumulator_set<qreal, stats<tag::rolling_sum, tag::rolling_count> > accumulator;
};


KisRollingSumAccumulatorWrapper::KisRollingSumAccumulatorWrapper(int windowSize)
    : m_d(new Private(windowSize))
{
}

KisRollingSumAccumulatorWrapper::~KisRollingSumAccumulatorWrapper()
{
}

void KisRollingSumAccumulatorWrapper::operator()(qreal value)
{
    m_d->accumulator(value);
}

qreal KisRollingSumAccumulatorWrapper::rollingSum() const
{
    return boost::accumulators::rolling_sum(m_d->accumulator);
}

int KisRollingSumAccumulatorWrapper::rollingCount() const
{
    return boost::accumulators::rolling_count(m_d->accumulator);
}

void KisRollingSumAccumulatorWrapper::reset(int windowSize)
{
    m_d->accumulator =
        accumulator_set<qreal, stats<tag::rolling_sum, tag::rolling_count>>(
            tag::rolling_window::window_size = windowSize);
}

