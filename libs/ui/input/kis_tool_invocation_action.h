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

#ifndef KIS_TOOL_INVOCATION_ACTION_H
#define KIS_TOOL_INVOCATION_ACTION_H

#include "kis_abstract_input_action.h"

/**
 * \brief Tool Invocation action of KisAbstractInputAction.
 *
 * The Tool Invocation action invokes the current tool, for example,
 * using the brush tool, it will start painting.
 */
class KisToolInvocationAction : public KisAbstractInputAction
{
public:
    enum Shortcut {
        ActivateShortcut,
        ConfirmShortcut,
        CancelShortcut,
        LineToolShortcut
    };
    explicit KisToolInvocationAction();
    ~KisToolInvocationAction() override;

    void activate(int shortcut) override;
    void deactivate(int shortcut) override;

    int priority() const override;
    bool canIgnoreModifiers() const override;

    void begin(int shortcut, QEvent *event) override;
    void end(QEvent *event) override;
    void inputEvent(QEvent* event) override;

    void processUnhandledEvent(QEvent* event);

    bool supportsHiResInputEvents(int shortcut) const override;

    bool isShortcutRequired(int shortcut) const override;

private:
    class Private;
    Private * const d;
};

#endif // KISTOOLINVOCATIONACTION_H
