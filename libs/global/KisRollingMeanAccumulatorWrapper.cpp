/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
