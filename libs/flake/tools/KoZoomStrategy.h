/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#ifndef KOZOOMSTRATEGY_H
#define KOZOOMSTRATEGY_H

#include "KoShapeRubberSelectStrategy.h"

class KoCanvasController;
class KoZoomTool;

/**
 * //internal
 * This is a strategy for the KoZoomTool which will be used to do the actual zooming
 */
class KoZoomStrategy : public KoShapeRubberSelectStrategy
{
public:
    /**
     * constructor
     * @param tool the parent tool this strategy is for
     * @param controller the canvas controller that wraps the canvas the tool is acting on.
     * @param clicked the location (in document points) where the interaction starts.
     */
    KoZoomStrategy(KoZoomTool *tool, KoCanvasController *controller, const QPointF &clicked);

    void forceZoomOut();
    void forceZoomIn();

    /// Execute the zoom
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;
    void cancelInteraction() override;

protected:
    SelectionMode currentMode() const override;
private:
    KoCanvasController *m_controller;

    bool m_forceZoomOut;
    Q_DECLARE_PRIVATE(KoShapeRubberSelectStrategy)
};

#endif

