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

#ifndef KOSHAPEAPPLICATIONDATA_H
#define KOSHAPEAPPLICATIONDATA_H

#include "flake_export.h"

/**
 * The KoShapeAppliationData class is used to associate application specific data with a shape.
 * See also the KoShapeUserData class that is specifically set for the benefit of shapes and tools.
 *
 * KoShapeAppliationData provides an abstract interface for container classes
 * that are used to associate application-specific data with shapes in KoShape
 * Generally, subclasses of this class provide functions to allow data to
 * be stored and retrieved, and instances are attached to KoShape using
 * KoShape::setApplicationData(). This makes it possible for an application to attach
 * application specific data (like a Frame in KWord) and have the deletion of a shape also delete
 * that data.
 * Each subclass should provide a reimplementation of the destructor to ensure that
 * any private data is automatically cleaned up when user data objects are deleted.
 */
class FLAKE_EXPORT KoShapeApplicationData
{
public:
    virtual ~KoShapeApplicationData();
};

#endif
