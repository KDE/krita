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
#ifndef LIBKIS_SHAPE_H
#define LIBKIS_SHAPE_H

#include <QObject>
#include <KoShape.h>

#include "kritalibkis_export.h"
#include "libkis.h"
#include <kis_types.h>

/**
 * @brief The Shape class
 * The shape class is a wrapper around Krita's vector objects.
 */
class KRITALIBKIS_EXPORT Shape : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Shape)

public:
    explicit Shape(KoShape *shape, QObject *parent = 0);
    ~Shape();
public Q_SLOTS:

    /**
     * @brief name
     * @return the name of the shape
     */
    QString name() const;

    /**
     * @brief setName
     * @param name which name the shape should have.
     */
    void setName(const QString &name);

    /**
     * @brief type
     * @return the type of shape.
     */
    virtual QString type() const;

    /**
     * @brief visible
     * @return whether the shape is visible.
     */
    bool visible();

    /**
     * @brief setVisible
     * @param visible whether the shape should be visible.
     */
    void setVisible(bool visible);

private:
    friend class GroupShape;
    friend class VectorLayer;

    struct Private;
    Private *const d;

    KoShape *shape();
};

#endif // LIBKIS_SHAPE_H
