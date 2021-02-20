/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
