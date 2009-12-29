/*
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
 * Copyright (C) Lukas Tvrdy <lukast.dev@gmail.com>, (C) 2009
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

#include "kis_qpainter_canvas.h"


#include <QPaintEvent>
#include <QPoint>
#include <QRect>
#include <QPainter>
#include <QImage>
#include <QBrush>
#include <QColor>
#include <QString>
#include <QTime>
#include <QTimer>
#include <QPixmap>
#include <QApplication>
#include <QMenu>

#include <kis_debug.h>
#include <kxmlguifactory.h>

#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoShapeManager.h>
#include <KoZoomHandler.h>
#include <KoToolManager.h>
#include <KoToolProxy.h>

#include <kis_image.h>
#include <kis_layer.h>

#include "kis_view2.h"
#include <ko_favorite_resource_manager.h>
#include "kis_canvas2.h"
#include "kis_prescaled_projection.h"
#include "kis_config.h"
#include "kis_canvas_resource_provider.h"
#include "kis_doc2.h"
#include "kis_selection_manager.h"
#include "kis_selection.h"
#include "kis_perspective_grid_manager.h"
#include "kis_config_notifier.h"

//#define DEBUG_REPAINT
#include <KoCanvasController.h>

class KisQPainterCanvas::Private
{
public:
    Private(const KoViewConverter *vc)
            : toolProxy(0),
            canvas(0),
            viewConverter(vc) {
    }

    KisPrescaledProjectionSP prescaledProjection;
    KoToolProxy * toolProxy;
    KisCanvas2 * canvas;
    const KoViewConverter * viewConverter;
    QBrush checkBrush;
    /// the offset of the view in the document, expressed in the view reference (not in the document reference)
    QPoint documentOffset;
    /// the origin of the image rect
    QPoint origin;
    QTimer blockMouseEvent;
};

KisQPainterCanvas::KisQPainterCanvas(KisCanvas2 * canvas, QWidget * parent)
        : QWidget(parent)
        , m_d(new Private(canvas->viewConverter()))
{
    // XXX: Reset pattern size and color when the properties change!

    KisConfig cfg;

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));

    setAutoFillBackground(true);
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled, true);
    setAttribute(Qt::WA_StaticContents);
    setAttribute(Qt::WA_OpaquePaintEvent);

    m_d->canvas =  canvas;
    m_d->toolProxy = canvas->toolProxy();
    m_d->checkBrush = QBrush(checkImage(cfg.checkSize()));
    m_d->blockMouseEvent.setSingleShot(true);


}

KisQPainterCanvas::~KisQPainterCanvas()
{
    delete m_d;
}

void KisQPainterCanvas::setPrescaledProjection(KisPrescaledProjectionSP prescaledProjection)
{
    m_d->prescaledProjection = prescaledProjection;
}

void KisQPainterCanvas::paintEvent(QPaintEvent * ev)
{
    KisConfig cfg;

    KisImageWSP image = m_d->canvas->image();
    if (image == 0) return;

    setAutoFillBackground(false);
#ifdef INDEPENDENT_CANVAS
    if (m_buffer.size() != size()) {
        m_buffer = QImage(size(), QImage::Format_ARGB32_Premultiplied);
    }
#endif

    QPoint documentOffset = m_d->documentOffset;

#ifdef INDEPENDENT_CANVAS
    QPainter gc(&m_buffer);
#else
    QPainter gc(this);
#endif

    gc.setCompositionMode(QPainter::CompositionMode_Source);
    gc.fillRect(QRect(QPoint(0, 0), size()), borderColor());

    // Don't draw the checks if we draw a cached pixmap, because we
    // need alpha transparency for checks. The precached pixmap
    // already should contain checks.
    QRect documentRect = QRect(QPoint(0, 0), documentSize());
    QRect fillRect = documentRect.translated(m_d->origin);

    if (!cfg.noXRender()) {

        if (cfg.scrollCheckers()) {


            if (documentOffset.x() > 0) {
                fillRect.adjust(0, 0, documentOffset.x(), 0);
            } else {
                fillRect.adjust(documentOffset.x(), 0, 0, 0);
            }
            if (documentOffset.y() > 0) {
                fillRect.adjust(0, 0, 0, documentOffset.y());
            } else {
                fillRect.adjust(0, documentOffset.y(), 0, 0);
            }
            gc.save();
            gc.translate(-documentOffset);
            gc.fillRect(fillRect, m_d->checkBrush);
            gc.restore();
        } else {
            // Checks
            gc.fillRect(fillRect, m_d->checkBrush);
        }
    }

    gc.setCompositionMode(QPainter::CompositionMode_SourceOver);
    if (cfg.noXRender()) {
        gc.drawPixmap(ev->rect(), m_d->prescaledProjection->prescaledPixmap(), ev->rect().translated(-m_d->origin));
    } else {
        gc.drawImage(ev->rect(), m_d->prescaledProjection->prescaledQImage(), ev->rect().translated(-m_d->origin));
    }

#ifdef DEBUG_REPAINT
    QColor color = QColor(random() % 255, random() % 255, random() % 255, 150);
    gc.fillRect(ev->rect(), color);
#endif
    drawDecorations(gc, true, m_d->documentOffset, fillRect.translated(-m_d->origin), m_d->canvas);
    gc.end();

#ifdef INDEPENDENT_CANVAS
    QPainter painter(this);
    painter.drawImage(ev->rect(), m_buffer, ev->rect());
#endif
}


void KisQPainterCanvas::enterEvent(QEvent* e)
{
    QWidget::enterEvent(e);
}

void KisQPainterCanvas::leaveEvent(QEvent* e)
{
    QWidget::leaveEvent(e);
}

void KisQPainterCanvas::mouseMoveEvent(QMouseEvent *e)
{
    if (m_d->blockMouseEvent.isActive()) return;
    m_d->toolProxy->mouseMoveEvent(e, m_d->viewConverter->viewToDocument(widgetToView(e->pos() + m_d->documentOffset)));
}

void KisQPainterCanvas::contextMenuEvent(QContextMenuEvent *e)
{
    m_d->canvas->view()->unplugActionList("flake_tool_actions");
    m_d->canvas->view()->plugActionList("flake_tool_actions",
                                        m_d->toolProxy->popupActionList());
    QMenu *menu = dynamic_cast<QMenu*>(m_d->canvas->view()->factory()->container("image_popup", m_d->canvas->view()));
    if (menu)
        menu->exec(e->globalPos());
}

void KisQPainterCanvas::mousePressEvent(QMouseEvent *e)
{
    if (m_d->blockMouseEvent.isActive()) return;
    else if (m_d->canvas->view()->favoriteResourceManager()->isPopupPaletteVisible())
    {
        m_d->canvas->view()->favoriteResourceManager()->slotShowPopupPalette();
        return;
    }
    m_d->toolProxy->mousePressEvent(e, m_d->viewConverter->viewToDocument(widgetToView(e->pos() + m_d->documentOffset)));
}

void KisQPainterCanvas::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_d->blockMouseEvent.isActive()) return;
    m_d->toolProxy->mouseReleaseEvent(e, m_d->viewConverter->viewToDocument(widgetToView(e->pos() + m_d->documentOffset)));
}

void KisQPainterCanvas::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (m_d->blockMouseEvent.isActive()) return;
    m_d->toolProxy->mouseDoubleClickEvent(e, m_d->viewConverter->viewToDocument(widgetToView(e->pos() + m_d->documentOffset)));
}

void KisQPainterCanvas::keyPressEvent(QKeyEvent *e)
{
    m_d->toolProxy->keyPressEvent(e);
    if (! e->isAccepted()) {
        if (e->key() == Qt::Key_Backtab
                || (e->key() == Qt::Key_Tab && (e->modifiers() & Qt::ShiftModifier)))
            focusNextPrevChild(false);
        else if (e->key() == Qt::Key_Tab)
            focusNextPrevChild(true);
    }
}

void KisQPainterCanvas::keyReleaseEvent(QKeyEvent *e)
{
    m_d->toolProxy->keyReleaseEvent(e);
}

QVariant KisQPainterCanvas::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return m_d->toolProxy->inputMethodQuery(query, *m_d->viewConverter);
}

void KisQPainterCanvas::inputMethodEvent(QInputMethodEvent *event)
{
    m_d->toolProxy->inputMethodEvent(event);
}

void KisQPainterCanvas::tabletEvent(QTabletEvent *e)
{
    setFocus(Qt::OtherFocusReason);
    m_d->blockMouseEvent.start(100);

    m_d->toolProxy->tabletEvent(e, m_d->viewConverter->viewToDocument(e->hiResGlobalPos() - mapToGlobal(QPoint(0, 0)) + m_d->documentOffset - m_d->origin));
}

void KisQPainterCanvas::wheelEvent(QWheelEvent *e)
{
    m_d->toolProxy->wheelEvent(e, m_d->viewConverter->viewToDocument(widgetToView(e->pos() + m_d->documentOffset)));
}

KoToolProxy * KisQPainterCanvas::toolProxy()
{
    return m_d->toolProxy;
}

void KisQPainterCanvas::documentOffsetMoved(const QPoint & pt)
{
    m_d->documentOffset = pt;
    m_d->prescaledProjection->documentOffsetMoved(pt);
    update();
}

void KisQPainterCanvas::resizeEvent(QResizeEvent *e)
{
    QSize size(e->size());
    if (size.width() <= 0) {
        size.setWidth(1);
    }
    if (size.height() <= 0) {
        size.setHeight(1);
    }
    m_d->prescaledProjection->resizePrescaledImage(size);
    adjustOrigin();
}

void KisQPainterCanvas::slotConfigChanged()
{
    KisConfig cfg;

    m_d->checkBrush = QBrush(checkImage(cfg.checkSize()));
}


void KisQPainterCanvas::adjustOrigin()
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
    int widthDiff = size().width() - documentRect.width();
    if (widthDiff > 0)
        m_d->origin.rx() += qRound(0.5 * widthDiff);
    int heightDiff = size().height() - documentRect.height();
    if (heightDiff > 0)
        m_d->origin.ry() += qRound(0.5 * heightDiff);


    emit documentOriginChanged(m_d->origin);
}


QPoint KisQPainterCanvas::documentOrigin()
{
    return m_d->origin;
}


QPoint KisQPainterCanvas::widgetToView(const QPoint& p) const
{
    return p - m_d->origin;
}


QRect KisQPainterCanvas::widgetToView(const QRect& r) const
{
    return r.translated(- m_d->origin);
}


QPoint KisQPainterCanvas::viewToWidget(const QPoint& p) const
{
    return p + m_d->origin;
}


QRect KisQPainterCanvas::viewToWidget(const QRect& r) const
{
    return r.translated(m_d->origin);
}


QSize KisQPainterCanvas::documentSize()
{
    KisImageWSP image = m_d->canvas->image();
    return QSize(int(ceil(m_d->viewConverter->documentToViewX(image->width()  / image->xRes()))),
                 int(ceil(m_d->viewConverter->documentToViewY(image->height() / image->yRes()))));
}


#include "kis_qpainter_canvas.moc"
