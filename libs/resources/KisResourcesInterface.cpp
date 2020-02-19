/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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

    auto it = d->sourceAdapters.find(type);
    if (it != d->sourceAdapters.end()) {
        return *(it->second);
    }

    ResourceSourceAdapter *source = createSourceImpl(type);
    KIS_ASSERT(source);

    std::unique_ptr<ResourceSourceAdapter> sourcePtr(source);
    d->sourceAdapters.emplace(type, std::move(sourcePtr));

    return *source;
}

KisResourcesInterface::ResourceSourceAdapter::ResourceSourceAdapter()
{
}

KisResourcesInterface::ResourceSourceAdapter::~ResourceSourceAdapter()
{
}
