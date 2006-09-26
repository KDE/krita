/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KoHistogramProducer.h"
#include "KoBasicHistogramProducers.h"
//Added by qt3to4:
#include <Q3ValueList>

KoHistogramProducerFactoryRegistry* KoHistogramProducerFactoryRegistry::m_singleton = 0;

KoHistogramProducerFactoryRegistry::KoHistogramProducerFactoryRegistry() {
    Q_ASSERT(KoHistogramProducerFactoryRegistry::m_singleton == 0);
}

KoHistogramProducerFactoryRegistry::~KoHistogramProducerFactoryRegistry() {
}

KoHistogramProducerFactoryRegistry* KoHistogramProducerFactoryRegistry::instance() {
    if(KoHistogramProducerFactoryRegistry::m_singleton == 0) {
        KoHistogramProducerFactoryRegistry::m_singleton
                = new KoHistogramProducerFactoryRegistry();
        m_singleton->add( new KoGenericLabHistogramProducerFactory() );
    }
    return KoHistogramProducerFactoryRegistry::m_singleton;
}

QList<KoID> KoHistogramProducerFactoryRegistry::listKeysCompatibleWith(
        KoColorSpace* colorSpace) const
{
    QList<KoID> list;
    Q3ValueList<float> preferredList;
    storageMap::const_iterator it = m_storage.begin();
    storageMap::const_iterator endit = m_storage.end();
    // O(n^2), can't this be done better? (But preferrably not by looking up the preferredness
    // during the sorting...
    while( it != endit ) {
        if (it->second->isCompatibleWith(colorSpace)) {
            float preferred = it->second->preferrednessLevelWith(colorSpace);
            Q3ValueList<float>::iterator pit = preferredList.begin();
            Q3ValueList<float>::iterator pend = preferredList.end();
            QList<KoID>::iterator lit = list.begin();

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
