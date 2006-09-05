/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KOSHAPEREGISTRY_H
#define KOSHAPEREGISTRY_H

#include <QObject>

#include <KoGenericRegistry.h>

#include <koffice_export.h>

class KoShape;
class KoShapeFactory;

/**
 * This singleton class keeps a register of all available flake shapes,
 * or rather, of the factories that applications can use to create flake
 * shape objects.
 */
class FLAKE_EXPORT KoShapeRegistry : public QObject,  public KoGenericRegistry<KoShapeFactory*>
{

    Q_OBJECT

public:

    virtual ~KoShapeRegistry();

    /**
     * Return an instance of the KoShapeRegistry
     * Creates an instance if that has never happened before and returns the singleton instance.
     */
    static KoShapeRegistry * instance();

private:
    KoShapeRegistry();
    KoShapeRegistry(const KoShapeRegistry&);
    KoShapeRegistry operator=(const KoShapeRegistry&);
    void init();

private:
    static KoShapeRegistry *m_singleton;
};

#endif
