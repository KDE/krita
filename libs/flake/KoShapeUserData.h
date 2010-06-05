/* This file is part of the KDE project
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

#ifndef KOSHAPEUSERDATA_H
#define KOSHAPEUSERDATA_H

#include <QObject>

#include "flake_export.h"

/**
 * The KoShapeUserData class is used to associate custom data with a shape.
 *
 *  KoShapeUserData provides an abstract interface for container classes
 *  that are used to associate application-specific user data with shapes in KoShape
 *  Generally, subclasses of this class provide functions to allow data to
 *  be stored and retrieved, and instances are attached to KoShape using
 *  KoShape::setUserData(). This makes it possible to store additional data per
 *  shape in a way that allows applications to not know the implementation of a
 *  specific KoShape extending class.
 *
 *  Each subclass should provide a reimplementation of the destructor to ensure that
 *  any private data is automatically cleaned up when user data objects are deleted.
 *
 *  Please note that this object is a QObject to allow a
 *  <code>qobject_cast<MyData*> (shape->userData())</code> to work which is useful in an environment
 *  where classes from plugins may not be castable using a static_cast or a dynamic_cast
 */
class FLAKE_EXPORT KoShapeUserData : public QObject
{
    Q_OBJECT
public:
    /// Constructor
    KoShapeUserData(QObject *parent = 0);
    virtual ~KoShapeUserData();
};

#endif
