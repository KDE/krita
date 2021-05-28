/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    void cursorMovedAbsolute(const QPointF &startPos, const QPointF &pos) override;
    void inputEvent(QEvent* event) override;

    KisInputActionGroup inputActionGroup(int shortcut) const override;
    bool supportsHiResInputEvents(int shortcut) const override;

private:
    class Private;
    Private * const d;
};

#endif // KIS_ROTATE_CANVAS_ACTION_H
