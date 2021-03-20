/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPEAPPLICATIONDATA_H
#define KOSHAPEAPPLICATIONDATA_H

#include "kritaflake_export.h"

/**
 * The KoShapeAppliationData class is used to associate application specific data with a shape.
 * See also the KoShapeUserData class that is specifically set for the benefit of shapes and tools.
 *
 * KoShapeAppliationData provides an abstract interface for container classes
 * that are used to associate application-specific data with shapes in KoShape
 * Generally, subclasses of this class provide functions to allow data to
 * be stored and retrieved, and instances are attached to KoShape using
 * KoShape::setApplicationData(). This makes it possible for an application to attach
 * application specific data (like a Frame in Words) and have the deletion of a shape also delete
 * that data.
 * Each subclass should provide a reimplementation of the destructor to ensure that
 * any private data is automatically cleaned up when user data objects are deleted.
 */
class KRITAFLAKE_EXPORT KoShapeApplicationData
{
public:
    virtual ~KoShapeApplicationData();
};

#endif
