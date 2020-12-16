/*  This file is part of the KDE project

    SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KORESOURCESERVEROBSERVER_H
#define KORESOURCESERVEROBSERVER_H

#include "kritawidgets_export.h"
#include <QSharedPointer>

/**
 * The KoResourceServerObserver class provides a interface to observe a KoResourceServer.
 * To receive notifications it needs to be added to the resource server.
 */
template <class T>
class KoResourceServerObserver
{
public:
    virtual ~KoResourceServerObserver() {}

    virtual void unsetResourceServer() = 0;

    /**
     * Will be called by the resource server after a resource is added
     * @param resource the added resource
     */
    virtual void resourceAdded(QSharedPointer<T> resource) = 0;

    /**
     * Will be called by the resource server before a resource will be removed
     * @param resource the resource which is going to be removed
     */
    virtual void removingResource(QSharedPointer<T> resource) = 0;

    /**
     * Will be called by the resource server when a resource is changed
     * @param resource the resource which is going to be removed
     */
    virtual void resourceChanged(QSharedPointer<T> resource) = 0;

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
