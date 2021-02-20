/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    bool supportsHiResInputEvents() const override;

    bool isShortcutRequired(int shortcut) const override;

private:
    class Private;
    Private * const d;
};

#endif // KISTOOLINVOCATIONACTION_H
