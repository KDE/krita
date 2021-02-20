/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
 * current tool. For example, using the brush tool it samples a color from
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
        SampleColorFgLayerModeShortcut,
        SampleColorBgLayerModeShortcut,
        SampleColorFgImageModeShortcut,
        SampleColorBgImageModeShortcut,
        TertiaryAlternateModeShortcut ///< Warning: don't reorder the items of this enum, it breaks user configs!
    };

    explicit KisAlternateInvocationAction();
    ~KisAlternateInvocationAction() override;

    void activate(int shortcut) override;
    void deactivate(int shortcut) override;

    int priority() const override;

    void begin(int shortcut, QEvent *event) override;
    void end(QEvent *event) override;
    void inputEvent(QEvent* event) override;

private:
    KisTool::ToolAction shortcutToToolAction(int shortcut);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_ALTERNATE_INVOCATION_ACTION_H
