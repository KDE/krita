/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_ROTATE_CANVAS_ACTION_H
#define KIS_ROTATE_CANVAS_ACTION_H

#include "kis_abstract_input_action.h"

/**
 * \brief Rotate Canvas implementation of KisAbstractInputAction.
 *
 * The Rotate Canvas action rotates the canvas.
 */

class KisRotateCanvasAction : public KisAbstractInputAction
{

public:
    /**
     * The different behaviours for this action.
     */
    enum Shortcut {
        RotateModeShortcut, ///< Toggle Rotate mode.
        DiscreteRotateModeShortcut, ///< Toggle Discrete Rotate mode.
        RotateLeftShortcut, ///< Rotate left by a fixed amount.
        RotateRightShortcut, ///< Rotate right by a fixed amount.
        RotateResetShortcut ///< Reset the rotation to 0.
    };
    explicit KisRotateCanvasAction();
    ~KisRotateCanvasAction() override;

    int priority() const override;

    void activate(int shortcut) override;
    void deactivate(int shortcut) override;
    void begin(int shortcut, QEvent *event) override;
    void cursorMoved(const QPointF &lastPos, const QPointF &pos) override;
    void inputEvent(QEvent* event) override;

    KisInputActionGroup inputActionGroup(int shortcut) const override;
    bool supportsHiResInputEvents() const;

private:
    class Private;
    Private * const d;
};

#endif // KIS_ROTATE_CANVAS_ACTION_H
