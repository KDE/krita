/* This file is part of the KDE project
 * Copyright ( C ) 2010 Boudewijn Rempt <boud@valdyas.org>
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

#include <KoPAViewBase.h>
#include <KoZoomHandler.h>

class KoPAViewBase::Private {

public:

    KoZoomHandler zoomHandler;
    KoPAViewMode * viewMode;
};

KoPAViewBase::KoPAViewBase()
    : d(new Private)
{
    d->viewMode = 0;
    proxyObject = new KoPAViewProxyObject(this);
}

KoPAViewBase::~KoPAViewBase()
{
    delete d;
    delete proxyObject;
}

KoViewConverter* KoPAViewBase::viewConverter( KoPACanvasBase * canvas )
{
    Q_UNUSED( canvas );

    return &d->zoomHandler;
}

KoZoomHandler* KoPAViewBase::zoomHandler()
{
    return &d->zoomHandler;
}

KoViewConverter* KoPAViewBase::viewConverter() const
{
    return &d->zoomHandler;
}

KoZoomHandler* KoPAViewBase::zoomHandler() const
{
    return &d->zoomHandler;
}

void KoPAViewBase::setViewMode( KoPAViewMode* mode )
{
    Q_ASSERT( mode );
    if ( !d->viewMode ) {
        d->viewMode = mode;
    }
    else if ( mode != d->viewMode ) {
        KoPAViewMode * previousViewMode = d->viewMode;
        d->viewMode->deactivate();
        d->viewMode = mode;
        d->viewMode->activate( previousViewMode );
    }
}

KoPAViewMode* KoPAViewBase::viewMode() const
{
    return d->viewMode;
}


KoPAViewProxyObject::KoPAViewProxyObject(KoPAViewBase *parent)
{
    m_view = parent;
}


#include <KoPAViewBase.moc>
