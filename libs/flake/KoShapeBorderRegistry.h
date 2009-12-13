/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef KOSHAPEBORDERREGISTRY_H
#define KOSHAPEBORDERREGISTRY_H

#include "flake_export.h"

#include <KoGenericRegistry.h>
#include <KoShapeBorderFactory.h>

#include <QObject>

class FLAKE_EXPORT KoShapeBorderRegistry : public QObject,  public KoGenericRegistry<KoShapeBorderFactory*>
{
    Q_OBJECT

public:
    ~KoShapeBorderRegistry();

    /**
     * Return an instance of the KoShapeBorderRegistry
     * Creates an instance if that has never happened before and returns the singleton instance.
     */
    static KoShapeBorderRegistry *instance();

private:
    KoShapeBorderRegistry();
    KoShapeBorderRegistry(const KoShapeBorderRegistry&);
    KoShapeBorderRegistry operator=(const KoShapeBorderRegistry&);
    void init();
};

#endif
