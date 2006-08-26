/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "krs_histogram.h"

#include <kis_paint_layer.h>

using namespace Kross::KritaCore;

Histogram::Histogram(KisPaintLayerSP layer,
                     KisHistogramProducerSP producer,
                     const enumHistogramType type)
    : QObject()
{
    setObjectName("KritaHistogram");
    m_histogram = new KisHistogram(layer, producer, type);
}

Histogram::~Histogram()
{
}

void Histogram::setChannel(uint channelnr)
{
    m_histogram->setChannel(channelnr);
}

uint Histogram::getChannel()
{
    return m_histogram->channel();
}

double Histogram::getMax()
{
    return m_histogram->calculations().getMax();
}

double Histogram::getMin()
{
    return m_histogram->calculations().getMin();
}

uint Histogram::getHighest()
{
    return m_histogram->calculations().getHighest();
}

uint Histogram::getLowest()
{
    return m_histogram->calculations().getLowest();
}

double Histogram::getMean()
{
    return m_histogram->calculations().getMean();
}

uint Histogram::getCount()
{
    return m_histogram->calculations().getCount();
}

double Histogram::getTotal()
{
    return m_histogram->calculations().getTotal();
}

uint Histogram::getValue(int index)
{
    return m_histogram->getValue(index);
}

int Histogram::getNumberOfBins()
{
    return m_histogram->producer()->numberOfBins();
}

#include "krs_histogram.moc"
