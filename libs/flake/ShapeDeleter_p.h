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
#ifndef SHAPEDELETER_H
#define SHAPEDELETER_H

#include <QObject>

class KoShape;

/**
 * Small helper class that will try to delete itself and the param shape with it.
 * It will do the actual delete in the main-thread, no matter which thread it has been
 * created in.
 * \internal
 */
class ShapeDeleter : public QObject
{
    Q_OBJECT
public:
    /**
     * Create a ShapeDeleter that will delete param shape
     * @param shape the shape that will be deleted soon after this object is created.
     */
    ShapeDeleter(KoShape *shape);
    ~ShapeDeleter();

private:
    KoShape *m_shape;
};

#endif
