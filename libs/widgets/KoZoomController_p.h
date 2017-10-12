/* This file is part of the KDE project
 * Copyright (C) 2007 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2010 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2011 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KoZoomController_p_h
#define KoZoomController_p_h

#include <KoZoomController.h>

#include <klocalizedstring.h>
#include <WidgetsDebug.h>

#include <KoZoomHandler.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>

class Q_DECL_HIDDEN KoZoomController::Private
{
public:
    Private(KoZoomController *p, KoZoomAction::SpecialButtons specialButtons)
        : canvasController(0), zoomHandler(0), action(0), textMinX(1), textMaxX(600), fitMargin(0), parent(p)
    {
        action = new KoZoomAction(KoZoomMode::ZOOM_WIDTH | KoZoomMode::ZOOM_PAGE, i18n("Zoom"), p);
        action->setSpecialButtons(specialButtons);
    }
    ~Private()
    {
    }

    /// so we know when the canvasController changes size
    void setAvailableSize()
    {
        if(zoomHandler->zoomMode() == KoZoomMode::ZOOM_WIDTH)
            setZoom(KoZoomMode::ZOOM_WIDTH, -1);
        if(zoomHandler->zoomMode() == KoZoomMode::ZOOM_PAGE)
            setZoom(KoZoomMode::ZOOM_PAGE, -1);
        if(zoomHandler->zoomMode() == KoZoomMode::ZOOM_TEXT)
            setZoom(KoZoomMode::ZOOM_TEXT, -1);
    }

    /// when the canvas controller wants us to change zoom
    void requestZoomRelative(const qreal factor, const QPointF& stillPoint)
    {
        parent->setZoom(KoZoomMode::ZOOM_CONSTANT, factor * zoomHandler->zoom(), stillPoint);
    }

    void setZoom(KoZoomMode::Mode mode, qreal zoom)
    {
        parent->setZoom(mode, zoom);
    }

    void init(KoCanvasController *co,
              KoZoomHandler *zh,
              KActionCollection *actionCollection);

    KoCanvasController *canvasController;
    KoZoomHandler *zoomHandler;
    KoZoomAction *action;
    QSizeF pageSize;
    qreal textMinX;
    qreal textMaxX;
    QSizeF documentSize;
    int fitMargin;
    KoZoomController *parent;
};

#endif
