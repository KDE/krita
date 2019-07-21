/*
 * This file is part of the KDE project
 * Copyright (C) 2019 Sharaf Zaman <sharafzaz121@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_ZOOM_AND_ROTATE_ACTION_H
#define KIS_ZOOM_AND_ROTATE_ACTION_H

#include "kis_abstract_input_action.h"


/**
 * @brief This class merely deligates the actions to KisZoomAction
 * _and_ KisRotateCanvasAction at the same time.
 */
class KisZoomAndRotateAction : public KisAbstractInputAction
{
public:
    KisZoomAndRotateAction();
    int priority() const override;

    void activate(int shortcut) override;
    void deactivate(int shortcut) override;
    void begin(int shortcut, QEvent *event) override;
    void cursorMovedAbsolute(const QPointF &lastPos, const QPointF &pos) override;
    void inputEvent(QEvent* event) override;

    KisInputActionGroup inputActionGroup(int shortcut) const override;
private:
    class Private;
    const Private *d;
};

#endif // KIS_ZOOM_AND_ROTATE_ACTION_H
