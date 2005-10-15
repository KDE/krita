/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include "kis_accumulating_producer.h"

KisAccumulatingHistogramProducer::KisAccumulatingHistogramProducer(KisCachedHistogramObserver::Producers* source)
    : KisBasicHistogramProducer(
        KisID("ACCHISTO", ""),
        source -> at(0) -> channels().count(),
        source -> at(0) -> numberOfBins(),
        0),
      m_source(source)
{
}

void KisAccumulatingHistogramProducer::addRegionToBin(Q_UINT8 *, Q_UINT8*, Q_UINT32, KisColorSpace*) {
    uint count = m_source -> count();
    for (uint i = 0; i < count; i++) {
        KisHistogramProducer* p = m_source -> at(i);

        for (int j = 0; j < m_channels; j++) {
            for (int k = 0; k < m_nrOfBins; k++) {
                m_bins.at(j).at(k) += p -> getBinAt(j, k);
            }
        }
    }
}
