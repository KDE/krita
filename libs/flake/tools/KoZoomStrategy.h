/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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

