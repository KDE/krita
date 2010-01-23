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

#ifndef KOPALOADINGCONTEXT_H
#define KOPALOADINGCONTEXT_H

#include <KoShapeLoadingContext.h>

#include <QMap>

#include "kopageapp_export.h"

class KoPAMasterPage;
class KoPAPage;

/// Context needed for loading the data of a pageapp
class KOPAGEAPP_EXPORT KoPALoadingContext : public KoShapeLoadingContext
{
public:
    /**
     * Constructor.
     *
     * @param context Context for loading oasis docs.
     * @param context The shape controller.
     */
     KoPALoadingContext(KoOdfLoadingContext &context, KoResourceManager *documentResources);
     ~KoPALoadingContext();

    /**
     * Get the master page with the name @p name.
     *
     * @param name name of the master page.
     */
    KoPAMasterPage* masterPageByName( const QString& name );

    /**
     * Add a master page to the context.
     *
     * @param name name of the master page.
     * @param master master page to add.
     */
    void addMasterPage( const QString& name, KoPAMasterPage* master );

    /**
     * Get the master pages
     */
    const QMap<QString, KoPAMasterPage *> & masterPages();

    /**
     * Get the page with the name @p name.
     *
     * @param name name of the page.
     */
    KoPAPage* pageByName( const QString& name );

    /**
     * Add a master page to the context.
     *
     * @param name name of the page.
     * @param page page to add.
     */
    void addPage( const QString& name, KoPAPage* page );

private:
    class Private;
    Private * const d;
};

#endif /*KOPALOADINGCONTEXT_H*/
