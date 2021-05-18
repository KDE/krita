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
    bool supportsHiResInputEvents(int shortcut) const override;

private:
    class Private;
    Private * const d;
};

#endif // KIS_PAN_ACTION_H
