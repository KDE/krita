/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "KoColorTransformationFactoryRegistry.h"

#include "KoColorTransformationFactory.h"

struct Q_DECL_HIDDEN KoColorTransformationFactoryRegistry::Private {
    static KoColorTransformationFactoryRegistry* s_registry;
};

KoColorTransformationFactoryRegistry* KoColorTransformationFactoryRegistry::Private::s_registry = 0;

KoColorTransformationFactoryRegistry::KoColorTransformationFactoryRegistry() : d(new Private)
{

}

KoColorTransformationFactoryRegistry::~KoColorTransformationFactoryRegistry()
{
    qDeleteAll(doubleEntries());
    qDeleteAll(values());
    delete d;
}


void KoColorTransformationFactoryRegistry::addColorTransformationFactory(KoColorTransformationFactory* factory)
{
    instance()->add(factory);
}

void KoColorTransformationFactoryRegistry::removeColorTransformationFactory(KoColorTransformationFactory* factory)
{
    instance()->remove(factory->id());
}

KoColorTransformationFactoryRegistry* KoColorTransformationFactoryRegistry::instance()
{
    if (Private::s_registry == 0) {
        Private::s_registry = new KoColorTransformationFactoryRegistry();
    }
    return Private::s_registry;
}
