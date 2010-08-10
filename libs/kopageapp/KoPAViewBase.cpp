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
};

KoPAViewBase::KoPAViewBase()
    : d(new Private)
{
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

KoPAViewProxyObject::KoPAViewProxyObject(KoPAViewBase *parent)
{
    m_view = parent;
}


#include <KoPAViewBase.moc>
