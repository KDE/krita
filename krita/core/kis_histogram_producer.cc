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
#include "color_strategy/kis_basic_histogram_producers.h"

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
        m_singleton->add( new KisGenericLightnessHistogramProducerFactory() );
    }
    return KisHistogramProducerFactoryRegistry::m_singleton;
}

KisIDList KisHistogramProducerFactoryRegistry::listKeysCompatibleWith(
        KisColorSpace* colorSpace) const
{
    KisIDList list;
    storageMap::const_iterator it = m_storage.begin();
    storageMap::const_iterator endit = m_storage.end();
    while( it != endit ) {
        if (it -> second -> isCompatibleWith(colorSpace)) {
            list.append(it -> first);
        }
        ++it;
    }
    return list;
}
