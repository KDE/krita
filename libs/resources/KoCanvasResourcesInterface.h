/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KOCANVASRESOURCESINTERFACE_H
#define KOCANVASRESOURCESINTERFACE_H

#include "kritaresources_export.h"
#include <QSharedPointer>

class QVariant;

#include <kritaresources_export.h>

/**
 * @brief An abstract class for providing access to canvas resources
 * like current gradient and Fg/Bg colors.
 *
 * Specific implementations may forward the requests either to
 * KoCanvasResourceProvider or to a local storage.
 */
class KRITARESOURCES_EXPORT KoCanvasResourcesInterface
{
public:
    virtual ~KoCanvasResourcesInterface();

    virtual QVariant resource(int key) const = 0;
};

using KoCanvasResourcesInterfaceSP = QSharedPointer<KoCanvasResourcesInterface>;

#endif // KOCANVASRESOURCESINTERFACE_H
