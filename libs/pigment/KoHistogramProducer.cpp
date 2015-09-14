/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KoHistogramProducer.h"

#include <QList>
#include <QGlobalStatic>

#include <KoID.h>

#include "KoBasicHistogramProducers.h"

#include "KoColorSpace.h"

Q_GLOBAL_STATIC(KoHistogramProducerFactoryRegistry, s_instance)

KoHistogramProducerFactoryRegistry::KoHistogramProducerFactoryRegistry()
{
}

KoHistogramProducerFactoryRegistry::~KoHistogramProducerFactoryRegistry()
{
    qDeleteAll(values());
}

KoHistogramProducerFactoryRegistry* KoHistogramProducerFactoryRegistry::instance()
{
    return s_instance;

}

QList<QString> KoHistogramProducerFactoryRegistry::keysCompatibleWith(const KoColorSpace* colorSpace) const
{
    QList<QString> list;
    QList<float> preferredList;
    foreach(const QString &id, keys()) {
        KoHistogramProducerFactory *f = value(id);
        if (f->isCompatibleWith(colorSpace)) {
            float preferred = f->preferrednessLevelWith(colorSpace);
            QList<float>::iterator pit = preferredList.begin();
            QList<float>::iterator pend = preferredList.end();
            QList<QString>::iterator lit = list.begin();

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
