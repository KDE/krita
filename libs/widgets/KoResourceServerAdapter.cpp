/*  This file is part of the KDE project

    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "KoResourceServerAdapter.h"

KoAbstractResourceServerAdapter::KoAbstractResourceServerAdapter(QObject *parent)
    : QObject(parent)
{
}

KoAbstractResourceServerAdapter::~KoAbstractResourceServerAdapter()
{
}

void KoAbstractResourceServerAdapter::emitResourceAdded(KoResource* resource)
{
    Q_EMIT resourceAdded(resource);
}

void KoAbstractResourceServerAdapter::emitRemovingResource(KoResource* resource)
{
    Q_EMIT removingResource(resource);
}

void KoAbstractResourceServerAdapter::emitResourceChanged(KoResource* resource)
{
    Q_EMIT resourceChanged(resource);
}

void KoAbstractResourceServerAdapter::emitTagsWereChanged()
{
    Q_EMIT tagsWereChanged();
}

void KoAbstractResourceServerAdapter::emitTagCategoryWasAdded(const QString& tag)
{
    Q_EMIT tagCategoryWasAdded(tag);
}

void KoAbstractResourceServerAdapter::emitTagCategoryWasRemoved(const QString& tag)
{
    Q_EMIT tagCategoryWasRemoved(tag);
}
