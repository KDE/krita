/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2019 Sharaf Zaman <sharafzaz121@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <klocalizedstring.h>
#include <kis_config.h>

#include "kis_zoom_and_rotate_action.h"
#include "kis_zoom_action.h"
#include "kis_rotate_canvas_action.h"

class KisZoomAndRotateAction::Private {
public:
    Private(): zoomAction(new KisZoomAction), rotateAction(new KisRotateCanvasAction) {}
    QScopedPointer<KisZoomAction> zoomAction;
    QScopedPointer<KisRotateCanvasAction> rotateAction;
};

KisZoomAndRotateAction::KisZoomAndRotateAction()
    : KisAbstractInputAction ("Zoom and Rotate Canvas")
    , d(new Private)
{
    setName(i18n("Zoom and Rotate Canvas"));
}

KisZoomAndRotateAction::~KisZoomAndRotateAction()
{
}

int KisZoomAndRotateAction::priority() const
{
    // if rotation is disabled, we set the lowest priority, so that, this action isn't triggered
    return KisConfig(true).disableTouchRotation() ? 0 : 5;
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

void KisZoomAndRotateAction::cursorMovedAbsolute(const QPointF &lastPos, const QPointF &pos)
{
    d->zoomAction->cursorMovedAbsolute(lastPos, pos);
    d->rotateAction->cursorMovedAbsolute(lastPos, pos);
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
