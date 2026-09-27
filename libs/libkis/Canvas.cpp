/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "Canvas.h"
#include <KoCanvasBase.h>
#include <kis_canvas2.h>
#include <KisView.h>
#include <KoCanvasController.h>
#include <kis_canvas_controller.h>
#include <kis_zoom_manager.h>
#include <View.h>

struct Canvas::Private {
    Private() {}
    KisCanvas2 *canvas {0};
    KoCanvasControllerProxyObject* canvasControllerProxy;
};

Canvas::Canvas(KoCanvasBase *canvas, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->canvas = static_cast<KisCanvas2*>(canvas);

    if (d->canvas) {
        d->canvasControllerProxy = d->canvas->imageView()->canvasController()->proxyObject;

        connect(d->canvasControllerProxy, SIGNAL(canvasRemoved(KoCanvasController*)), this, SIGNAL(removed()));
        connect(d->canvasControllerProxy, SIGNAL(canvasOffsetChanged()), this, SIGNAL(offsetChanged()));
        connect(d->canvasControllerProxy, SIGNAL(canvasMousePositionChanged(QPoint)), this, SIGNAL(cursorPositionChanged(QPoint)));
        connect(d->canvasControllerProxy, SIGNAL(documentMousePositionChanged(QPointF)), this, SIGNAL(documentCursorPositionChanged(QPointF)));
        connect(d->canvasControllerProxy, SIGNAL(sizeChanged(QSize)), this, SIGNAL(sizeChanged(QSize)));
        connect(d->canvasControllerProxy, SIGNAL(moveDocumentOffset(QPointF, QPointF)), this, SIGNAL(documentOffsetChanged(QPointF, QPointF)));
        connect(d->canvasControllerProxy, SIGNAL(effectiveZoomChanged(qreal)), this, SIGNAL(effectiveZoomChanged(qreal)));
        connect(d->canvasControllerProxy, SIGNAL(zoomStateChanged(KoZoomState)), this, SLOT(emitZoomStateChanged(KoZoomState)));
        connect(d->canvasControllerProxy, SIGNAL(documentRectInWidgetPixelsChanged(QRectF)), this, SIGNAL(documentRectInWidgetPixelsChanged(QRectF)));
        connect(d->canvasControllerProxy, SIGNAL(documentRotationChanged(qreal)), this, SIGNAL(rotationChanged(qreal)));
        connect(d->canvasControllerProxy, SIGNAL(documentMirrorStatusChanged(bool, bool)), this, SIGNAL(mirrorChanged(bool, bool)));
        connect(d->canvasControllerProxy, SIGNAL(canvasStateChanged()), this, SIGNAL(stateChanged()));
    }
}

Canvas::~Canvas()
{
    delete d;
}


bool Canvas::operator==(const Canvas &other) const
{
    return (d->canvas == other.d->canvas);
}

bool Canvas::operator!=(const Canvas &other) const
{
    return !(operator==(other));
}


qreal Canvas::zoomLevel() const
{
    if (!d->canvas) return 1.0;
    return d->canvas->imageView()->viewConverter()->zoom();
}

void Canvas::setZoomLevel(qreal value)
{
    if (!d->canvas) return;
    d->canvas->imageView()->canvasController()->setZoom(KoZoomMode::ZOOM_CONSTANT, value);
}

void Canvas::setPreferredCenter(const QPointF& imagePos)
{
    if (!d->canvas) return;

    const KisCoordinatesConverter *coordConv = d->canvas->coordinatesConverter();

    QPointF documentPos = coordConv->imageToDocument(imagePos);
    QPointF flakePos = coordConv->documentToFlake(documentPos);

    d->canvas->imageView()->canvasController()->setPreferredCenter(flakePos);
}

QPointF Canvas::preferredCenter() const
{
    if (!d->canvas) return QPointF();

    QPointF flakePos = d->canvas->imageView()->canvasController()->preferredCenter();

    const KisCoordinatesConverter *coordConv = d->canvas->coordinatesConverter();
    QPointF docPos = coordConv->flakeToDocument(flakePos);
    QPointF imagePos = coordConv->documentToImage(docPos);

    return imagePos; 
}

void Canvas::pan(const QPoint& offset)
{
    d->canvas->imageView()->canvasController()->pan(offset);
}

void Canvas::resetZoom()
{
    if (!d->canvas) return;
    d->canvas->imageView()->zoomManager()->zoomTo100();
}


void Canvas::resetRotation()
{
    if (!d->canvas) return;
    d->canvas->imageView()->canvasController()->resetCanvasRotation();
}

qreal Canvas::rotation() const
{
    if (!d->canvas) return 0;
    return d->canvas->imageView()->canvasController()->rotation();
}

void Canvas::setRotation(qreal angle)
{
    if (!d->canvas) return;
    d->canvas->imageView()->canvasController()->rotateCanvas(angle - rotation());
}


bool Canvas::mirror() const
{
    if (!d->canvas) return false;
    return d->canvas->imageView()->canvasIsMirrored();
}

void Canvas::setMirror(bool value)
{
    if (!d->canvas) return;
    d->canvas->imageView()->canvasController()->mirrorCanvas(value);
}

View *Canvas::view() const
{
    if (!d->canvas) return 0;
    View *view = new View(d->canvas->imageView());
    return view;
}

KisDisplayColorConverter *Canvas::displayColorConverter() const
{
    if (!d->canvas) return 0;
    return d->canvas->displayColorConverter();
}

bool Canvas::wrapAroundMode() const
{
    if (!d->canvas) return false;
    return d->canvas->imageView()->canvasController()->wrapAroundMode();
}

void Canvas::setWrapAroundMode(bool enable)
{
    if (!d->canvas) return;
    d->canvas->imageView()->canvasController()->slotToggleWrapAroundMode(enable);
}

bool Canvas::levelOfDetailMode() const
{
    if (!d->canvas) return false;
    return d->canvas->imageView()->canvasController()->levelOfDetailMode();
}

void Canvas::setLevelOfDetailMode(bool enable)
{
    if (!d->canvas) return;
    return d->canvas->imageView()->canvasController()->slotToggleLevelOfDetailMode(enable);
}

void Canvas::emitZoomStateChanged(const KoZoomState &zoomState)
{
    emit zoomStateChanged((ZoomMode)zoomState.mode, zoomState.zoom, zoomState.minZoom, zoomState.maxZoom);
}

#include "moc_Canvas.cpp"
