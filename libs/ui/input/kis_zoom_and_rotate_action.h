/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2019 Sharaf Zaman <sharafzaz121@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_ZOOM_AND_ROTATE_ACTION_H
#define KIS_ZOOM_AND_ROTATE_ACTION_H

#include "kis_abstract_input_action.h"
#include <QScopedPointer>

/**
 * @brief This class merely deligates the actions to KisZoomAction
 * _and_ KisRotateCanvasAction at the same time.
 */
class KisZoomAndRotateAction : public KisAbstractInputAction
{
public:
    KisZoomAndRotateAction();
    ~KisZoomAndRotateAction();

    int priority() const override;

    void activate(int shortcut) override;
    void deactivate(int shortcut) override;
    void begin(int shortcut, QEvent *event) override;
    void cursorMovedAbsolute(const QPointF &lastPos, const QPointF &pos) override;
    void inputEvent(QEvent* event) override;

    KisInputActionGroup inputActionGroup(int shortcut) const override;
private:
    class Private;
    const QScopedPointer<Private> d;
};

#endif // KIS_ZOOM_AND_ROTATE_ACTION_H
