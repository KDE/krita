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

#include <klocalizedstring.h>

#include "kis_zoom_and_rotate_action.h"
#include "kis_zoom_action.h"
#include "kis_rotate_canvas_action.h"

class KisZoomAndRotateAction::Private {
public:
    Private(): zoomAction(new KisZoomAction), rotateAction(new KisRotateCanvasAction) {}
    KisZoomAction *zoomAction;
    KisRotateCanvasAction *rotateAction;
};

KisZoomAndRotateAction::KisZoomAndRotateAction()
    : KisAbstractInputAction ("Zoom and Rotate Canvas")
    , d(new Private)
{
    setName(i18n("Zoom and Rotate Canvas"));
}

int KisZoomAndRotateAction::priority() const
{
    return 5;
}

void KisZoomAndRotateAction::activate(int shortcut)
{
    d->zoomAction->activate(shortcut);
    d->rotateAction->activate(shortcut);
}

void KisZoomAndRotateAction::deactivate(int shortcut)
{
    d->zoomAction->deactivate(shortcut);
    d->rotateAction->deactivate(shortcut);
}

void KisZoomAndRotateAction::begin(int shortcut, QEvent *event)
{
    d->zoomAction->begin(shortcut, event);
    d->rotateAction->begin(shortcut, event);
}

void KisZoomAndRotateAction::cursorMoved(const QPointF &lastPos, const QPointF &pos)
{
    d->zoomAction->cursorMoved(lastPos, pos);
    d->rotateAction->cursorMoved(lastPos, pos);
}

void KisZoomAndRotateAction::inputEvent(QEvent *event)
{
    d->zoomAction->inputEvent(event);
    d->rotateAction->inputEvent(event);
}

KisInputActionGroup KisZoomAndRotateAction::inputActionGroup(int shortcut) const
{
    Q_UNUSED(shortcut);
    return ViewTransformActionGroup;
}
