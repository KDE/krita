/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KISRESOURCELOADERREGISTRY_H
#define KISRESOURCELOADERREGISTRY_H

#include <KoGenericRegistry.h>
#include <QObject>
#include "KisResourceLoader.h"

#include <kritaresources_export.h>

class KRITARESOURCES_EXPORT KisResourceLoaderRegistry : public QObject, public KoGenericRegistry<KisResourceLoaderBase*>
{
    Q_OBJECT
public:
    ~KisResourceLoaderRegistry() override;

    static KisResourceLoaderRegistry* instance();

private:

    KisResourceLoaderRegistry(QObject *parent);
    KisResourceLoaderRegistry(const KisResourceLoaderRegistry&);
    KisResourceLoaderRegistry operator=(const KisResourceLoaderRegistry&);
};

#endif // KISRESOURCELOADERREGISTRY_H
