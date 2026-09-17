/*
 *  SPDX-FileCopyrightText: 2026 Ayanami Kaine <personal@ayanamikaine.com>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TEMPORARY_STRAIGHT_RULER_ACTION_H
#define KIS_TEMPORARY_STRAIGHT_RULER_ACTION_H

#include "kis_abstract_input_action.h"

class KisTemporaryStraightRulerAction : public KisAbstractInputAction
{
public:
    enum Shortcut {
        NormalShortcut,
        SnapToAngleShortcut
    };

    KisTemporaryStraightRulerAction();
    ~KisTemporaryStraightRulerAction() override;

    int priority() const override;
    void activate(int shortcut) override;
    void deactivate(int shortcut) override;
    bool trySwitchShortcut(int oldShortcut, int newShortcut) override;

    void begin(int shortcut, QEvent *event) override;
    void inputEvent(QEvent *event) override;
    void end(QEvent *event) override;

    bool supportsHiResInputEvents(int shortcut) const override;
    bool isAvailable() const override;
    KisInputActionGroup inputActionGroup(int shortcut) const override;

private:
    class Private;
    Private * const d;
};

#endif // KIS_TEMPORARY_STRAIGHT_RULER_ACTION_H
