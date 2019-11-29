/* This file is part of the KDE project
 *
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
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

#include "KoShapePainter.h"

#include "KoCanvasBase.h"
#include "KoSelectedShapesProxySimple.h"
#include "KoShapeManager.h"
#include "KoShape.h"
#include "KoShapeStrokeModel.h"
#include "KoShapeGroup.h"
#include "KoShapeContainer.h"
#include <KoViewConverter.h>
#include <KoUnit.h>

#include <QPainter>
#include <QImage>

class SimpleCanvas : public KoCanvasBase
{
public:
    SimpleCanvas()
        : KoCanvasBase(0),
          m_shapeManager(new KoShapeManager(this)),
          m_selectedShapesProxy(new KoSelectedShapesProxySimple(m_shapeManager.data()))
    {
    }

    ~SimpleCanvas() override
    {
    }

    void gridSize(QPointF *offset, QSizeF *spacing) const override
    {
        *offset = QPointF();
        *spacing = QSizeF();
    };

    bool snapToGrid() const override
    {
        return false;
    }

    void addCommand(KUndo2Command *) override
    {
    }

    KoShapeManager *shapeManager() const override
    {
        return m_shapeManager.data();
    }

    KoSelectedShapesProxy *selectedShapesProxy() const override
    {
        return m_selectedShapesProxy.data();
    }

    void updateCanvas(const QRectF&) override
    {
    }

    KoToolProxy *toolProxy() const override
    {
        return 0;
    }

    KoViewConverter *viewConverter() const override
    {
        return 0;
    }

    QWidget *canvasWidget() override
    {
        return 0;
    }

    const QWidget *canvasWidget() const override
    {
        return 0;
    }

    KoUnit unit() const override
    {
        return KoUnit(KoUnit::Point);
    }

    void updateInputMethodInfo() override {}

    void setCursor(const QCursor &) override {}

private:
    QScopedPointer<KoShapeManager> m_shapeManager;
    QScopedPointer<KoSelectedShapesProxySimple> m_selectedShapesProxy;
};

class Q_DECL_HIDDEN KoShapePainter::Private
{
public:
    Private()
        : canvas(new SimpleCanvas())
    {
    }

    ~Private() { delete canvas; }
    SimpleCanvas * canvas;
};

KoShapePainter::KoShapePainter()
    : d(new Private())
{
}

KoShapePainter::~KoShapePainter()
{
    delete d;
}

void KoShapePainter::setShapes(const QList<KoShape*> &shapes)
{
    d->canvas->shapeManager()->setShapes(shapes, KoShapeManager::AddWithoutRepaint);
}

void KoShapePainter::paint(QPainter &painter)
{
    foreach (KoShape *shape, d->canvas->shapeManager()->shapes()) {
        shape->waitUntilReady(false);
    }

    d->canvas->shapeManager()->paint(painter, true);
}

void KoShapePainter::paint(QPainter &painter, const QRect &painterRect, const QRectF &documentRect)
{
    if (documentRect.width() == 0.0f || documentRect.height() == 0.0f)
        return;

    KoViewConverter converter;
    // calculate the painter destination rectangle size in document coordinates
    QRectF paintBox = converter.viewToDocument(QRectF(QPointF(), painterRect.size()));

    // compute the zoom factor based on the bounding rects in document coordinates
    // so that the content fits into the image
    qreal zoomW = paintBox.width() / documentRect.width();
    qreal zoomH = paintBox.height() / documentRect.height();
    qreal zoom = qMin(zoomW, zoomH);

    // now set the zoom into the zoom handler used for painting the shape
    converter.setZoom(zoom);

    painter.save();

    // initialize painter
    painter.setPen(QPen(Qt::NoPen));
    painter.setBrush(Qt::NoBrush);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(painterRect.adjusted(-1,-1,1,1));

    // convert document rectangle to view coordinates
    QRectF zoomedBound = converter.documentToView(documentRect);
    // calculate offset between painter rectangle and converted document rectangle
    QPointF offset = QRectF(painterRect).center() - zoomedBound.center();
    // center content in painter rectangle
    painter.translate(offset.x(), offset.y());
    painter.setTransform(converter.documentToView(), true);

    // finally paint the shapes
    paint(painter);

    painter.restore();
}

void KoShapePainter::paint(QImage &image)
{
    if (image.isNull())
        return;

    QPainter painter(&image);

    paint(painter, image.rect(), contentRect());
}

QRectF KoShapePainter::contentRect() const
{
    QRectF bound;
    foreach (KoShape *shape, d->canvas->shapeManager()->shapes()) {
        if (!shape->isVisible())
            continue;
        if (dynamic_cast<KoShapeGroup*>(shape))
            continue;

        QRectF shapeRect = shape->boundingRect();

        if (bound.isEmpty())
            bound = shapeRect;
        else
            bound = bound.united(shapeRect);
    }
    return bound;
}
