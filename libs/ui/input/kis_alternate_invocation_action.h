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

#ifndef KIS_ALTERNATE_INVOCATION_ACTION_H
#define KIS_ALTERNATE_INVOCATION_ACTION_H

#include "kis_abstract_input_action.h"
#include <QScopedPointer>
#include "kis_tool.h"

/**
 * \brief Alternate Invocation implementation of KisAbstractInputAction.
 *
 * The Alternate Invocation action performs an alternate action with the
 * current tool. For example, using the brush tool it picks a color from
 * the canvas.
 */
class KisAlternateInvocationAction : public KisAbstractInputAction
{
public:
    /**
     * The different behaviours for this action.
     */
    enum Shortcut {
        PrimaryAlternateModeShortcut, ///< Toggle Primary mode.
        SecondaryAlternateModeShortcut, ///< Toggle Secondary mode.
        PickColorFgLayerModeShortcut,
        PickColorBgLayerModeShortcut,
        PickColorFgImageModeShortcut,
        PickColorBgImageModeShortcut,
    };

    explicit KisAlternateInvocationAction();
    virtual ~KisAlternateInvocationAction();

    void activate(int shortcut);
    void deactivate(int shortcut);

    virtual int priority() const;

    void begin(int shortcut, QEvent *event);
    void end(QEvent *event);
    void inputEvent(QEvent* event);

private:
    KisTool::ToolAction shortcutToToolAction(int shortcut);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_ALTERNATE_INVOCATION_ACTION_H
