/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_GAMMA_EXPOSURE_ACTION_H
#define __KIS_GAMMA_EXPOSURE_ACTION_H

#include "kis_abstract_input_action.h"


class KisGammaExposureAction : public KisAbstractInputAction
{
public:
    /**
     * The different behaviours for this action.
     */
    enum Shortcuts {
        ExposureShortcut,
        GammaShortcut,
        AddExposure05Shortcut,
        RemoveExposure05Shortcut,
        AddGamma05Shortcut,
        RemoveGamma05Shortcut,
        AddExposure02Shortcut,
        RemoveExposure02Shortcut,
        AddGamma02Shortcut,
        RemoveGamma02Shortcut,
        ResetExposureAndGammaShortcut
    };
    explicit KisGammaExposureAction();
    ~KisGammaExposureAction() override;

    int priority() const override;

    void activate(int shortcut) override;
    void deactivate(int shortcut) override;

    void begin(int shortcut, QEvent *event = 0) override;
    void cursorMovedAbsolute(const QPointF &lastPos, const QPointF &pos) override;

    bool isShortcutRequired(int shortcut) const override;

private:
    class Private;
    Private * const d;
};

#endif /* __KIS_GAMMA_EXPOSURE_ACTION_H */
