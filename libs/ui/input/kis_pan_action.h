/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PAN_ACTION_H
#define KIS_PAN_ACTION_H

#include "kis_abstract_input_action.h"

/**
 * \brief Pan Canvas implementation of KisAbstractInputAction.
 *
 * The Pan Canvas action pans the canvas.
 */
class KisPanAction : public KisAbstractInputAction
{
public:
    /**
     * The different behaviours for this action.
     */
    enum Shortcut {
        PanModeShortcut, ///< Toggle the pan mode.
        PanLeftShortcut, ///< Pan left by a fixed amount.
        PanRightShortcut, ///< Pan right by a fixed amount.
        PanUpShortcut, ///< Pan up by a fixed amount.
        PanDownShortcut ///< Pan down by a fixed amount.
    };

    explicit KisPanAction();
    ~KisPanAction() override;

    int priority() const override;

    void activate(int shortcut) override;
    void deactivate(int shortcut) override;

    void begin(int shortcut, QEvent *event) override;
    void end(QEvent *event) override;

    void inputEvent(QEvent* event) override;
    void cursorMovedAbsolute(const QPointF &lastPos, const QPointF &pos) override;

    bool isShortcutRequired(int shortcut) const override;

    KisInputActionGroup inputActionGroup(int shortcut) const override;
    bool supportsHiResInputEvents() const override;

private:
    class Private;
    Private * const d;
};

#endif // KIS_PAN_ACTION_H
