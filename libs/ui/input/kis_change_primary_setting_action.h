/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCHANGEPRIMARYSETTINGACTION_H
#define KISCHANGEPRIMARYSETTINGACTION_H

#include "kis_abstract_input_action.h"
#include "kis_tool.h"

/**
 * \brief Change Primary Setting implementation of KisAbstractInputAction.
 *
 * The Change Primary Setting action changes a tool's "Primary Setting",
 * for example the brush size for the brush tool.
 */
class KisChangePrimarySettingAction : public KisAbstractInputAction
{
public:
    explicit KisChangePrimarySettingAction();
    ~KisChangePrimarySettingAction() override;

    enum Shortcut {
        PrimaryAlternateChangeSizeShortcut, ///< Default Mapping: Shift+Left Mouse
        SecondaryAlternateChangeSizeShortcut, ///< Secondary Mode (snap to closest pixel value): Shift+Z+Left Mouse
    };

    void activate(int shortcut) override;
    void deactivate(int shortcut) override;
    int priority() const override;

    void begin(int shortcut, QEvent *event) override;
    void end(QEvent *event) override;
    void inputEvent(QEvent* event) override;
private:
    KisTool::ToolAction decodeAction(int shorcut);
    KisTool::ToolAction savedAction;
};

#endif // KISCHANGEPRIMARYSETTINGACTION_H
