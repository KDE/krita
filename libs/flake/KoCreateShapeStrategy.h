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

#ifndef KOCREATESHAPESTRATEGY_H
#define KOCREATESHAPESTRATEGY_H

#include "KoShapeRubberSelectStrategy.h"

#include <QPointF>

class KoCanvasBase;
class KoCreateShapesTool;

/**
 * A strategy for the KoCreateShapesTool.
 */
class KoCreateShapeStrategy : public KoShapeRubberSelectStrategy {
public:
    /**
     * Constructor that starts to create a new shape.
     * @param tool the parent tool which controls this strategy
     * @param canvas the canvas interface which will supply things like a selection object
     * @param clicked the initial point that the user depressed (in pt).
     */
    KoCreateShapeStrategy( KoCreateShapesTool *tool, KoCanvasBase *canvas, const QPointF &clicked );
    virtual ~KoCreateShapeStrategy() {}

    void finishInteraction();
    KCommand* createCommand();

private:
    KoCreateShapesTool *m_tool;
};

#endif /* KOSHAPEROTATESTRATEGY_H */

