/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisResourcesInterface.h"


#include <QString>
#include "kis_assert.h"
#include "KisResourcesInterface_p.h"

#include "kis_debug.h"

KisResourcesInterface::KisResourcesInterface()
    : d_ptr(new KisResourcesInterfacePrivate)
{
}

KisResourcesInterface::KisResourcesInterface(KisResourcesInterfacePrivate *dd)
    : d_ptr(dd)
{
}

KisResourcesInterface::~KisResourcesInterface()
{

}

KisResourcesInterface::ResourceSourceAdapter &KisResourcesInterface::source(const QString &type) const
{
    Q_D(const KisResourcesInterface);

    // use double-locking for fetching the source

    ResourceSourceAdapter *source = 0;

    {
        QReadLocker l(&d->lock);
        source = d->findExistingSource(type);
        if (source) return *source;
    }

    {
        QWriteLocker l(&d->lock);
        source = d->findExistingSource(type);
        if (source) return *source;

        source = createSourceImpl(type);

        std::unique_ptr<ResourceSourceAdapter> sourcePtr(source);
        d->sourceAdapters.emplace(type, std::move(sourcePtr));
    }

    KIS_ASSERT(source);
    return *source;
}

KisResourcesInterface::ResourceSourceAdapter::ResourceSourceAdapter()
{
}

KisResourcesInterface::ResourceSourceAdapter::~ResourceSourceAdapter()
{
}
