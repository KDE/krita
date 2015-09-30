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

#ifndef KORESOURCESERVEROBSERVER_H
#define KORESOURCESERVEROBSERVER_H

#include "kritawidgets_export.h"

#include "KoResourceServerPolicies.h"


/**
 * The KoResourceServerObserver class provides a interface to observe a KoResourceServer.
 * To receive notifications it needs to be added to the resource server.
 */
template <class T, class Policy = PointerStoragePolicy<T> >
class KoResourceServerObserver
{
public:
    virtual ~KoResourceServerObserver() {}
    typedef typename Policy::PointerType PointerType;

    virtual void unsetResourceServer() = 0;

    /**
     * Will be called by the resource server after a resource is added
     * @param resource the added resource
     */
    virtual void resourceAdded(PointerType resource) = 0;

    /**
     * Will be called by the resource server before a resource will be removed
     * @param resource the resource which is going to be removed
     */
    virtual void removingResource(PointerType resource) = 0;

    /**
     * Will be called by the resource server when a resource is changed
     * @param resource the resource which is going to be removed
     */
    virtual void resourceChanged(PointerType resource) = 0;

     /**
     * Will be called by the resource server when resources are added or removed
     * from a tag category
     */
    virtual void syncTaggedResourceView()=0;

     /**
     * Will be called by the resource server when a new tag category has been created
     */
    virtual void syncTagAddition(const QString& tag)=0;

     /**
     * Will be called by the resource server when a new tag category has been deleted
     */
    virtual void syncTagRemoval(const QString& tag)=0;

};

#endif // KORESOURCESERVEROBSERVER_H
