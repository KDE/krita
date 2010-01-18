/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoColorSpaceFactory.h"
#include "KoColorProfile.h"
#include "KoColorSpace.h"

struct KoColorSpaceFactory::Private {
    QList<KoColorSpace*> colorspaces;
    QMap<QString, QList<KoColorSpace*> > availableColorspaces;
};

KoColorSpaceFactory::KoColorSpaceFactory() : d(new Private)
{
}

KoColorSpaceFactory::~KoColorSpaceFactory()
{
    // Check that all color spaces have been released
    int count = 0;
    foreach( const QList<KoColorSpace*>& cl, d->availableColorspaces)
    {
        count += cl.size();
    }
    Q_ASSERT(count == colorspaces.size());
    foreach( KoColorSpace* cs, d->colorspaces)
    {
        delete cs;
    }
    delete d;
}

KoColorSpace* KoColorSpaceFactory::grabColorSpace(const KoColorProfile * profile)
{
    QList<KoColorSpace*>& csList = d->availableColorspaces[profile->name()];
    if (!csList.isEmpty()) {
        KoColorSpace* cs = csList.back();
        csList.pop_back();
        return cs;
    }
    KoColorSpace* cs = createColorSpace(profile);
    d->colorspaces.push_back(cs);
    return cs;
}

void KoColorSpaceFactory::releaseColorSpace(KoColorSpace * colorspace)
{
    // TODO it is probably worth to avoid caching too many color spaces
    d->availableColorspaces[colorspace->profile()->name()].push_back(colorspace);
}
