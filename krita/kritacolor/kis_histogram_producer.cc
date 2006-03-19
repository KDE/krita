/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include "kis_histogram_producer.h"
#include "kis_basic_histogram_producers.h"

KisHistogramProducerFactoryRegistry* KisHistogramProducerFactoryRegistry::m_singleton = 0;

KisHistogramProducerFactoryRegistry::KisHistogramProducerFactoryRegistry() {
    Q_ASSERT(KisHistogramProducerFactoryRegistry::m_singleton == 0);
}

KisHistogramProducerFactoryRegistry::~KisHistogramProducerFactoryRegistry() {
}

KisHistogramProducerFactoryRegistry* KisHistogramProducerFactoryRegistry::instance() {
    if(KisHistogramProducerFactoryRegistry::m_singleton == 0) {
        KisHistogramProducerFactoryRegistry::m_singleton
                = new KisHistogramProducerFactoryRegistry();
        m_singleton->add( new KisGenericLabHistogramProducerFactory() );
    }
    return KisHistogramProducerFactoryRegistry::m_singleton;
}

KisIDList KisHistogramProducerFactoryRegistry::listKeysCompatibleWith(
        KisColorSpace* colorSpace) const
{
    KisIDList list;
    QValueList<float> preferredList;
    storageMap::const_iterator it = m_storage.begin();
    storageMap::const_iterator endit = m_storage.end();
    // O(n^2), can't this be done better? (But preferrably not by looking up the preferredness
    // during the sorting...
    while( it != endit ) {
        if (it->second->isCompatibleWith(colorSpace)) {
            float preferred = it->second->preferrednessLevelWith(colorSpace);
            QValueList<float>::iterator pit = preferredList.begin();
            QValueList<float>::iterator pend = preferredList.end();
            KisIDList::iterator lit = list.begin();

            while (pit != pend && preferred <= *pit) {
                ++pit;
                ++lit;
            }

            list.insert(lit, it->first);
            preferredList.insert(pit, preferred);
        }
        ++it;
    }
    return list;
}
