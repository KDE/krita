/* This file is part of the KDE project
 * Copyright (C) 2007 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (C) 2007-2009 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoPALoadingContext.h"

#include <QMap>

#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <KoOdfReadStore.h>
#include "KoPAMasterPage.h"
#include "KoPAPage.h"

class KoPALoadingContext::Private
{
public:
    QMap<QString, KoPAMasterPage*> masterPages;
    QMap<QString, KoPAPage*> pages;
};

KoPALoadingContext::KoPALoadingContext(KoOdfLoadingContext &context, KoResourceManager *documentResources)
: KoShapeLoadingContext(context, documentResources)
, d( new Private() )
{
}

KoPALoadingContext::~KoPALoadingContext()
{
    delete d;
}

KoPAMasterPage* KoPALoadingContext::masterPageByName( const QString& name )
{
    return d->masterPages.value( name, 0 );
}

void KoPALoadingContext::addMasterPage( const QString& name, KoPAMasterPage* master )
{
    d->masterPages.insert( name, master );
}

const QMap<QString, KoPAMasterPage *> & KoPALoadingContext::masterPages()
{
    return d->masterPages;
}

KoPAPage* KoPALoadingContext::pageByName( const QString& name )
{
    return d->pages.value( name, 0 );
}

void KoPALoadingContext::addPage( const QString& name, KoPAPage* page )
{
    d->pages.insert( name, page );
}
