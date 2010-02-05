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
#include <ko_favorite_resource_manager.h>

#include "kis_canvas_decoration.h"
#include "../kis_config.h"
#include "kis_canvas2.h"
#include "../kis_view2.h"
#include "../kis_selection_manager.h"

class KisCanvasWidgetBase::Private
{
public:
    Private(KisCanvas2 *newCanvas)
        : canvas(newCanvas)
        , viewConverter(newCanvas->viewConverter())
        , toolProxy(newCanvas->toolProxy())
    {}

    QList<KisCanvasDecoration*> decorations;
    KisCanvas2 * canvas;
    const KoViewConverter * viewConverter;
    KoToolProxy * toolProxy;
    /// the offset of the view in the document, expressed in the view reference (not in the document reference)
    QPoint documentOffset;
    /// the origin of the image rect
    QPoint origin;
    QTimer blockMouseEvent;
};

KisCanvasWidgetBase::KisCanvasWidgetBase(KisCanvas2 * canvas)
    : m_d(new Private(canvas))
{
    m_d->blockMouseEvent.setSingleShot(true);
}

KisCanvasWidgetBase::~KisCanvasWidgetBase()
{
    delete m_d;
}

void KisCanvasWidgetBase::drawDecorations(QPainter & gc, bool tools,
                                          const QPoint & documentOffset,
                                          const QRect & clipRect,
                                          KisCanvas2 * canvas)
{
    // Setup the painter to take care of the offset; all that the
    // classes that do painting need to keep track of is resolution
    gc.setRenderHint(QPainter::Antialiasing);
    gc.setRenderHint(QPainter::TextAntialiasing);
    gc.setRenderHint(QPainter::HighQualityAntialiasing);
    gc.setRenderHint(QPainter::SmoothPixmapTransform);
    gc.translate(QPoint(-documentOffset.x(), -documentOffset.y()));
    gc.translate(documentOrigin());

    // Paint the shapes (other than the layers)
    gc.save();
    gc.setClipRect(clipRect);
    canvas->globalShapeManager()->paint(gc, *canvas->viewConverter(), false);
    gc.restore();

    // ask the decorations to paint themselves
    foreach(KisCanvasDecoration* deco, m_d->decorations) {
        deco->paint(gc, documentOffset, clipRect, *canvas->viewConverter());
    }

    // Give the tool a chance to paint its stuff
    if (tools) {
        gc.save();
        toolProxy()->paint(gc, *canvas->viewConverter());
        gc.restore();
    }
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

    QImage tile(checkSize * 2, checkSize * 2, QImage::Format_RGB32);
    QPainter pt(&tile);
    pt.fillRect(tile.rect(), Qt::white);
    pt.fillRect(0, 0, checkSize, checkSize, cfg.checkersColor());
    pt.fillRect(checkSize, checkSize, checkSize, checkSize, cfg.checkersColor());
    pt.end();

    return tile;
}

QColor KisCanvasWidgetBase::borderColor() const
{
    return QColor(Qt::gray);
}

KisCanvas2 *KisCanvasWidgetBase::canvas() const
{
    return m_d->canvas;
}

const KoViewConverter *KisCanvasWidgetBase::viewConverter() const
{
    return m_d->viewConverter;
}

KoToolProxy *KisCanvasWidgetBase::toolProxy()
{
    return m_d->toolProxy;
}

QPoint KisCanvasWidgetBase::documentOffset() const
{
    return m_d->documentOffset;
}

QPoint KisCanvasWidgetBase::documentOrigin() const
{
    return m_d->origin;
}

void KisCanvasWidgetBase::documentOffsetMoved(const QPoint & pt)
{
    m_d->documentOffset = pt;
}

QSize KisCanvasWidgetBase::documentSize() const
{
    KisImageWSP image = m_d->canvas->image();
    return QSize(int(ceil(m_d->viewConverter->documentToViewX(image->width()  / image->xRes()))),
                 int(ceil(m_d->viewConverter->documentToViewY(image->height() / image->yRes()))));
}

void KisCanvasWidgetBase::adjustOrigin()
{
    KisImageWSP image = m_d->canvas->image();
    if (image == 0) return;

    QRect documentRect = QRect(QPoint(0, 0), documentSize());

    // save the old origin to see if it has changed
    QPoint oldOrigin = m_d->origin;

    // set the origin to the zoom document rect origin
    m_d->origin = -documentRect.topLeft();

    // the document bounding rect is always centered on the virtual canvas
    // if there are margins left around the zoomed document rect then
    // distribute them evenly on both sides
    int widthDiff = widget()->size().width() - documentRect.width();
    if (widthDiff > 0)
        m_d->origin.rx() += qRound(0.5 * widthDiff);
    int heightDiff = widget()->size().height() - documentRect.height();
    if (heightDiff > 0)
        m_d->origin.ry() += qRound(0.5 * heightDiff);
 
    emitDocumentOriginChangedSignal();
}


QPointF KisCanvasWidgetBase::widgetToDocument(const QPointF& p) const
{
    return m_d->viewConverter->viewToDocument(widgetToView(p + m_d->documentOffset));
}


QPointF KisCanvasWidgetBase::mouseEventWidgetToDocument(const QPoint& mousePosition) const
{
    const qreal PIXEL_CENTRE_OFFSET = 0.5;
    const QPointF pixelCentre(mousePosition.x() + PIXEL_CENTRE_OFFSET,
                              mousePosition.y() + PIXEL_CENTRE_OFFSET);
    return widgetToDocument(pixelCentre);
}


QPointF KisCanvasWidgetBase::widgetToView(const QPointF& p) const
{
    return p - m_d->origin;
}


QRect KisCanvasWidgetBase::widgetToView(const QRect& r) const
{
    return r.translated(- m_d->origin);
}


QPoint KisCanvasWidgetBase::viewToWidget(const QPoint& p) const
{
    return p + m_d->origin;
}


QRect KisCanvasWidgetBase::viewToWidget(const QRect& r) const
{
    return r.translated(m_d->origin);
}


void KisCanvasWidgetBase::processMouseMoveEvent(QMouseEvent *e)
{
    if (m_d->blockMouseEvent.isActive()) {
        return;
    }
    m_d->toolProxy->mouseMoveEvent(e, mouseEventWidgetToDocument(e->pos()));
}

void KisCanvasWidgetBase::processContextMenuEvent(QContextMenuEvent *e)
{
    m_d->canvas->view()->unplugActionList("flake_tool_actions");
    m_d->canvas->view()->plugActionList("flake_tool_actions",
                                        m_d->toolProxy->popupActionList());
    QMenu *menu = dynamic_cast<QMenu*>(m_d->canvas->view()->factory()->container("image_popup", m_d->canvas->view()));
    if (menu)
        menu->exec(e->globalPos());
}

void KisCanvasWidgetBase::processMousePressEvent(QMouseEvent *e)
{
    if (m_d->blockMouseEvent.isActive()) {
        return;
    } else if (m_d->canvas->view()->favoriteResourceManager()->isPopupPaletteVisible()) {
        m_d->canvas->view()->favoriteResourceManager()->slotShowPopupPalette();
        return;
    }
    m_d->toolProxy->mousePressEvent(e, mouseEventWidgetToDocument(e->pos()));
}

void KisCanvasWidgetBase::processMouseReleaseEvent(QMouseEvent *e)
{
    if (m_d->blockMouseEvent.isActive()) {
        return;
    }
    m_d->toolProxy->mouseReleaseEvent(e, mouseEventWidgetToDocument(e->pos()));
}

void KisCanvasWidgetBase::processMouseDoubleClickEvent(QMouseEvent *e)
{
    if (m_d->blockMouseEvent.isActive()) {
        return;
    }
    m_d->toolProxy->mouseDoubleClickEvent(e, mouseEventWidgetToDocument(e->pos()));
}

void KisCanvasWidgetBase::processKeyPressEvent(QKeyEvent *e)
{
    m_d->toolProxy->keyPressEvent(e);
    if (! e->isAccepted()) {
        if (e->key() == Qt::Key_Backtab
                || (e->key() == Qt::Key_Tab && (e->modifiers() & Qt::ShiftModifier)))
            callFocusNextPrevChild(false);
        else if (e->key() == Qt::Key_Tab)
            callFocusNextPrevChild(true);
    }
}

void KisCanvasWidgetBase::processKeyReleaseEvent(QKeyEvent *e)
{
    m_d->toolProxy->keyReleaseEvent(e);
}

QVariant KisCanvasWidgetBase::processInputMethodQuery(Qt::InputMethodQuery query) const
{
    return m_d->toolProxy->inputMethodQuery(query, *m_d->viewConverter);
}

void KisCanvasWidgetBase::processInputMethodEvent(QInputMethodEvent *event)
{
    m_d->toolProxy->inputMethodEvent(event);
}

void KisCanvasWidgetBase::processTabletEvent(QTabletEvent *e)
{
    widget()->setFocus(Qt::OtherFocusReason);
    m_d->blockMouseEvent.start(100);

    const QPointF pos = e->hiResGlobalPos() - widget()->mapToGlobal(QPoint(0, 0));
    m_d->toolProxy->tabletEvent(e, widgetToDocument(pos));
}

void KisCanvasWidgetBase::processWheelEvent(QWheelEvent *e)
{
    m_d->toolProxy->wheelEvent(e, widgetToDocument(e->pos()));
}


