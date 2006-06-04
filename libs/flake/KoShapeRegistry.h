/* 
 * KoShapeRegistry.h -- Part of KOffice
 *
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KOSHAPEREGISTRY_H
#define KOSHAPEREGISTRY_H

#include <QObject>

#include <KoGenericRegistry.h>

class KoShape;
class KoShapeFactory;

/**
 * This singleton class keeps a register of all available flake shapes,
 * or rather, of the factories that applications can use to create flake
 * shape objects.
 */
class KoShapeRegistry : public QObject,  public KoGenericRegistry<KoShapeFactory*>
{

    Q_OBJECT

public:

    virtual ~KoShapeRegistry();

    static KoShapeRegistry * instance();

private:
    KoShapeRegistry();
    KoShapeRegistry(const KoShapeRegistry&);
    KoShapeRegistry operator=(const KoShapeRegistry&);

private:
    static KoShapeRegistry *m_singleton;

    
};

#endif // FLAKESHAPES_H
