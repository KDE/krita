/*
 *  Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef LIBKIS_GROUPSHAPE_H
#define LIBKIS_GROUPSHAPE_H

#include "kritalibkis_export.h"
#include "libkis.h"
#include "Shape.h"
#include <kis_types.h>
#include <KoShapeGroup.h>

/**
 * @brief The GroupShape class
 * A group shape is a vector object with child shapes.
 */

class KRITALIBKIS_EXPORT GroupShape : public Shape
{
    Q_OBJECT

public:
    explicit GroupShape(QObject *parent = 0);
    GroupShape(KoShapeGroup *shape, QObject *parent = 0);
    ~GroupShape();
public Q_SLOTS:

    /**
     * @brief type returns the type.
     * @return "groupshape"
     */
    QString type() const override;

    /**
     * @brief children
     * @return the child shapes of this group shape.
     */
    QList<Shape*> children();
};

#endif // LIBKIS_GROUPSHAPE_H
