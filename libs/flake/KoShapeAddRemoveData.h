/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOSHAPEADDREMOVEDATA_H
#define KOSHAPEADDREMOVEDATA_H

#include <koffice_export.h>

/**
 * This is a virtual base class for an object that is passed to a KoShapeControllerBase
 * during add/remove of a shape. A application can use it it pass any information that 
 * can be used to add/remove a shape. 
 * Only if an application sets this in KoCanvasBase the information is passed.
 */
class FLAKE_EXPORT KoShapeAddRemoveData
{
public: 
    /**
     * @brief Constructor
     */
    KoShapeAddRemoveData() {}

    /**
     * @brief Destructor
     */
    virtual ~KoShapeAddRemoveData() {}

    /**
     * @brief Clone the object
     *
     * @return a copy of the object
     */
    virtual KoShapeAddRemoveData * clone() const = 0;
};

#endif /* KOSHAPEADDREMOVEDATA_H */
