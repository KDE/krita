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
    Private() {}

    KisPrescaledProjectionSP prescaledProjection;
    QBrush checkBrush;
};

KisQPainterCanvas::KisQPainterCanvas(KisCanvas2 * canvas, QWidget * parent)
        : QWidget(parent)
        , KisCanvasWidgetBase(canvas)
        , m_d(new Private())
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

    m_d->checkBrush = QBrush(checkImage(cfg.checkSize()));
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

    KisImageWSP image = canvas()->image();
    if (image == 0) return;

    setAutoFillBackground(false);
#ifdef INDEPENDENT_CANVAS
    if (m_buffer.size() != size()) {
        m_buffer = QImage(size(), QImage::Format_ARGB32_Premultiplied);
    }
#endif

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
    QRect fillRect = documentRect.translated(documentOrigin());

    if (!cfg.noXRender()) {

        if (cfg.scrollCheckers()) {


            if (documentOffset().x() > 0) {
                fillRect.adjust(0, 0, documentOffset().x(), 0);
            } else {
                fillRect.adjust(documentOffset().x(), 0, 0, 0);
            }
            if (documentOffset().y() > 0) {
                fillRect.adjust(0, 0, 0, documentOffset().y());
            } else {
                fillRect.adjust(0, documentOffset().y(), 0, 0);
            }
            gc.save();
            gc.translate(-documentOffset());
            gc.fillRect(fillRect, m_d->checkBrush);
            gc.restore();
        } else {
            // Checks
            gc.fillRect(fillRect, m_d->checkBrush);
        }
    }

    gc.setCompositionMode(QPainter::CompositionMode_SourceOver);
    if (cfg.noXRender()) {
        gc.drawPixmap(ev->rect(), m_d->prescaledProjection->prescaledPixmap(), 
                      ev->rect().translated(-documentOrigin()));
    } else {
        gc.drawImage(ev->rect(), m_d->prescaledProjection->prescaledQImage(), 
                     ev->rect().translated(-documentOrigin()));
    }

#ifdef DEBUG_REPAINT
    QColor color = QColor(random() % 255, random() % 255, random() % 255, 150);
    gc.fillRect(ev->rect(), color);
#endif
    drawDecorations(gc, true, documentOffset(), fillRect.translated(-documentOrigin()), canvas());
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
    processMouseMoveEvent(e);
}

void KisQPainterCanvas::contextMenuEvent(QContextMenuEvent *e)
{
    processContextMenuEvent(e);
}

void KisQPainterCanvas::mousePressEvent(QMouseEvent *e)
{
    processMousePressEvent(e);
}

void KisQPainterCanvas::mouseReleaseEvent(QMouseEvent *e)
{
    processMouseReleaseEvent(e);
}

void KisQPainterCanvas::mouseDoubleClickEvent(QMouseEvent *e)
{
    processMouseDoubleClickEvent(e);
}

void KisQPainterCanvas::keyPressEvent(QKeyEvent *e)
{
    processKeyPressEvent(e);
}

void KisQPainterCanvas::keyReleaseEvent(QKeyEvent *e)
{
    processKeyReleaseEvent(e);
}

QVariant KisQPainterCanvas::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return processInputMethodQuery(query);
}

void KisQPainterCanvas::inputMethodEvent(QInputMethodEvent *event)
{
    processInputMethodEvent(event);
}

void KisQPainterCanvas::tabletEvent(QTabletEvent *e)
{
    processTabletEvent(e);
}

void KisQPainterCanvas::wheelEvent(QWheelEvent *e)
{
    processWheelEvent(e);
}

void KisQPainterCanvas::documentOffsetMoved(const QPoint & pt)
{
    KisCanvasWidgetBase::documentOffsetMoved(pt);
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

void KisQPainterCanvas::emitDocumentOriginChangedSignal()
{
    emit documentOriginChanged(documentOrigin());
}

bool KisQPainterCanvas::callFocusNextPrevChild(bool next)
{
    return focusNextPrevChild(next);
}

#include "kis_qpainter_canvas.moc"
