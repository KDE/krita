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
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <QList>

#include <KoID.h>

#include "KoHistogramProducer.h"
#include "KoBasicHistogramProducers.h"

#include "KoColorSpace.h"

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
    QList<float> preferredList;
    foreach(KoID id, listKeys()) {
        KoHistogramProducerFactory *f = value(id.id());
        if (f->isCompatibleWith(colorSpace)) {
            float preferred = f->preferrednessLevelWith(colorSpace);
            QList<float>::iterator pit = preferredList.begin();
            QList<float>::iterator pend = preferredList.end();
            QList<KoID>::iterator lit = list.begin();

            while (pit != pend && preferred <= *pit) {
                ++pit;
                ++lit;
            }

            list.insert(lit, id);
            preferredList.insert(pit, preferred);
        }
    }
    return list;
}
