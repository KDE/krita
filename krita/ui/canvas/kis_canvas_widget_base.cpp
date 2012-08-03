/*
 * Copyright (C) 2007, 2010 Adrian Page <adrian@pagenet.plus.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_canvas_widget_base.h"

#include <QImage>
#include <QPainter>
#include <QTimer>
#include <QMenu>

#include <kxmlguifactory.h>

#include <KoShapeManager.h>
#include <KoViewConverter.h>
#include <KoToolProxy.h>
#include <KoCanvasController.h>
#include <KoShape.h>
#include <KoSelection.h>
#include <KoShapePaintingContext.h>

#include "kis_coordinates_converter.h"
#include "kis_canvas_decoration.h"
#include "../kis_config.h"
#include "kis_canvas2.h"
#include "../kis_view2.h"
#include "../kis_selection_manager.h"

struct KisCanvasWidgetBase::Private
{
public:
    Private(KisCanvas2 *newCanvas, KisCoordinatesConverter *newCoordinatesConverter)
        : canvas(newCanvas)
        , coordinatesConverter(newCoordinatesConverter)
        , viewConverter(newCanvas->viewConverter())
        , toolProxy(newCanvas->toolProxy())
        , ignorenextMouseEventExceptRightMiddleClick(0)
        , borderColor(Qt::gray)
    {}

    QList<KisCanvasDecoration*> decorations;
    KisCanvas2 * canvas;
    KisCoordinatesConverter *coordinatesConverter;
    const KoViewConverter * viewConverter;
    KoToolProxy * toolProxy;
    QTimer blockMouseEvent;

    bool ignorenextMouseEventExceptRightMiddleClick; // HACK work around Qt bug not sending tablet right/dblclick http://bugreports.qt.nokia.com/browse/QTBUG-8598
    QColor borderColor;
};

KisCanvasWidgetBase::KisCanvasWidgetBase(KisCanvas2 * canvas, KisCoordinatesConverter *coordinatesConverter)
    : m_d(new Private(canvas, coordinatesConverter))
{
    m_d->blockMouseEvent.setSingleShot(true);
}

KisCanvasWidgetBase::~KisCanvasWidgetBase()
{
    delete m_d;
}

void KisCanvasWidgetBase::drawDecorations(QPainter & gc, const QRect &updateWidgetRect)
{
    gc.save();

    // Setup the painter to take care of the offset; all that the
    // classes that do painting need to keep track of is resolution
    gc.setRenderHint(QPainter::Antialiasing);
    gc.setRenderHint(QPainter::TextAntialiasing);

    // This option does not do anything anymore with Qt4.6, so don't reenable it since it seems to break display
    // http://www.archivum.info/qt-interest@trolltech.com/2010-01/00481/Re:-(Qt-interest)-Is-QPainter::HighQualityAntialiasing-render-hint-broken-in-Qt-4.6.html
    // gc.setRenderHint(QPainter::HighQualityAntialiasing);

    gc.setRenderHint(QPainter::SmoothPixmapTransform);


    gc.save();
    gc.setClipRect(updateWidgetRect);

    QTransform transform = m_d->coordinatesConverter->flakeToWidgetTransform();
    gc.setTransform(transform);

    // Paint the shapes (other than the layers)
    m_d->canvas->globalShapeManager()->paint(gc, *m_d->viewConverter, false);


    // draw green selection outlines around text shapes that are edited, so the user sees where they end
    gc.save();
    QTransform worldTransform = gc.worldTransform();
    gc.setPen( Qt::green );

    foreach (KoShape *shape, canvas()->shapeManager()->selection()->selectedShapes()) {
        if (shape->shapeId() == "ArtisticText" || shape->shapeId() == "TextShapeID") {
            gc.setWorldTransform(shape->absoluteTransformation(m_d->viewConverter) * worldTransform);
            KoShape::applyConversion(gc, *m_d->viewConverter);
            gc.drawRect(QRectF(QPointF(), shape->size()));
        }
    }
    gc.restore();

    // Draw text shape over canvas while editing it, that's needs to show the text selection correctly
    QString toolId = KoToolManager::instance()->activeToolId();
    if (toolId == "ArtisticTextToolFactoryID" || toolId == "TextToolFactory_ID") {
        gc.save();
        gc.setPen(Qt::NoPen);
        gc.setBrush(Qt::NoBrush);
        foreach (KoShape *shape, canvas()->shapeManager()->selection()->selectedShapes()) {
            if (shape->shapeId() == "ArtisticText" || shape->shapeId() == "TextShapeID") {
                KoShapePaintingContext  paintContext(canvas(), false);
                gc.save();
                gc.setTransform(shape->absoluteTransformation(m_d->viewConverter) * gc.transform());
                canvas()->shapeManager()->paintShape(shape, gc, *m_d->viewConverter, paintContext);
                gc.restore();
            }
        }
        gc.restore();
    }

    // - some tools do not restore gc, but that is not important here
    // - we need to disable clipping to draw handles properly
    gc.setClipping(false);
    toolProxy()->paint(gc, *m_d->viewConverter);
    gc.restore();

    // ask the decorations to paint themselves
    foreach(KisCanvasDecoration* deco, m_d->decorations) {
        deco->paint(gc, m_d->coordinatesConverter->widgetToDocument(updateWidgetRect), m_d->coordinatesConverter);
    }

    gc.restore();
}

void KisCanvasWidgetBase::addDecoration(KisCanvasDecoration* deco)
{
    m_d->decorations.push_back(deco);
}

KisCanvasDecoration* KisCanvasWidgetBase::decoration(const QString& id)
{
    foreach(KisCanvasDecoration* deco, m_d->decorations) {
        if (deco->id() == id) {
            return deco;
        }
    }
    return 0;
}

void KisCanvasWidgetBase::setDecorations(const QList<KisCanvasDecoration*> &decorations)
{
    m_d->decorations=decorations;
}

QList<KisCanvasDecoration*> KisCanvasWidgetBase::decorations()
{
    return m_d->decorations;
}

QImage KisCanvasWidgetBase::checkImage(qint32 checkSize)
{
    KisConfig cfg;

    if(checkSize < 0)
        checkSize = cfg.checkSize();

    QColor checkColor = cfg.checkersColor();

    QImage tile(checkSize * 2, checkSize * 2, QImage::Format_RGB32);
    QPainter pt(&tile);
    pt.fillRect(tile.rect(), Qt::white);
    pt.fillRect(0, 0, checkSize, checkSize, checkColor);
    pt.fillRect(checkSize, checkSize, checkSize, checkSize, checkColor);
    pt.end();

    return tile;
}

void KisCanvasWidgetBase::notifyConfigChanged()
{
    KisConfig cfg;
    m_d->borderColor = cfg.canvasBorderColor();
}

QColor KisCanvasWidgetBase::borderColor() const
{
    return m_d->borderColor;
}

KisCanvas2 *KisCanvasWidgetBase::canvas() const
{
    return m_d->canvas;
}

KisCoordinatesConverter* KisCanvasWidgetBase::coordinatesConverter()
{
    return m_d->coordinatesConverter;
}

KoToolProxy *KisCanvasWidgetBase::toolProxy()
{
    return m_d->toolProxy;
}

QVariant KisCanvasWidgetBase::processInputMethodQuery(Qt::InputMethodQuery query) const
{
    if (query == Qt::ImMicroFocus) {
        QRectF rect = (m_d->toolProxy->inputMethodQuery(query, *m_d->viewConverter).toRectF()).toRect();
        QPointF scroll(m_d->canvas->view()->canvasController()->scrollBarValue());
        rect.translate(m_d->canvas->documentOrigin() - scroll);
        return rect.toRect();
    }
    return m_d->toolProxy->inputMethodQuery(query, *m_d->viewConverter);
}

void KisCanvasWidgetBase::processInputMethodEvent(QInputMethodEvent *event)
{
    m_d->toolProxy->inputMethodEvent(event);
}
