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

#ifndef KOSHAPESCALETRATEGY_H
#define KOSHAPESCALETRATEGY_H

#include "KoInteractionStrategy.h"
#include "KoInteractionTool.h"

#include <QPointF>

class KoCanvasBase;
class KoTool;

/**
 * A strategy for the KoInteractionTool.
 * This strategy is invoked when the user starts a resize of a selection of objects,
 * the stategy will then resize the objects interactively and provide a command afterwards.
 */
class KoShapeResizeStrategy : public KoInteractionStrategy
{
public:
    /**
     * Constructor
     */
    KoShapeResizeStrategy( KoTool *tool, KoCanvasBase *canvas, const QPointF &clicked, KoFlake::SelectionHandle direction );
    virtual ~KoShapeResizeStrategy() {}

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    KCommand* createCommand();
    void finishInteraction() { }
    virtual void paint( QPainter &painter, KoViewConverter &converter);

private:
    QPointF m_start;
    QList<QPointF> m_startPositions;
    QList<QPointF> m_startAbsolutePositions;
    QList<QSizeF> m_startSizes;
    QList<double> m_startShearXs;
    QList<double> m_startShearYs;
    bool m_top, m_left, m_bottom, m_right;
    QMatrix m_unwindMatrix, m_windMatrix;
    QSizeF m_initialSize;
    QPointF m_initialPosition;
};

#endif /* KOSHAPESCALETRATEGY_H */

