/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 C. Boemann <cbo@boemann.dk>
 * SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2010 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2011 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
    Private(KoZoomController *p)
        : canvasController(0), zoomHandler(0), action(0), textMinX(1), textMaxX(600), fitMargin(0), parent(p)
    {
        action = new KoZoomAction(KoZoomMode::ZOOM_WIDTH | KoZoomMode::ZOOM_PAGE | KoZoomMode::ZOOM_HEIGHT, i18n("Zoom"), p);
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
