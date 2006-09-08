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
#ifndef KWFRAMEGEOMETRY_H
#define KWFRAMEGEOMETRY_H

#include "ui_KoShapeGeometry.h"

#include <KoShapeConfigWidgetBase.h>

#include <QWidget>
#include <QList>

class KoShape;

/**
 * A widget that shows the basic geometry items of a shape.
 */
class KoShapeGeometry : public KoShapeConfigWidgetBase {
    Q_OBJECT
public:
    KoShapeGeometry();
    ~KoShapeGeometry();

    /// reimplemented from KoShapeConfigWidgetBase
    void open(KoShape *shape);
    /// reimplemented from KoShapeConfigWidgetBase
    void save();
    /// reimplemented from KoShapeConfigWidgetBase
    KAction *createAction();

    /// reimplemented from KoShapeConfigWidgetBase
    void setUnit(KoUnit::Unit unit);

private:
    Ui::KoShapeGeometry widget;
    KoShape *m_shape;
};

#endif
