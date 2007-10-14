/*
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
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

#include "kis_canvas2.h"

#include <QPaintEvent>
#include <QRect>
#include <QPainter>
#include <QImage>
#include <QBrush>
#include <QColor>
#include <QString>
#include <QTime>
#include <QPixmap>
#include <QApplication>
#include <QMenu>

#include <kdebug.h>
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


#include "kis_prescaled_projection.h"
#include "kis_config.h"
#include "kis_resource_provider.h"
#include "kis_doc2.h"
#include "kis_grid_drawer.h"
#include "kis_selection_manager.h"
#include "kis_selection.h"

//#define DEBUG_REPAINT
//#define USE_QT_SCALING


#define NOT_DEFAULT_EXPOSURE 1e100

class KisQPainterCanvas::Private {
public:
    Private(const KoViewConverter *vc)
        : toolProxy(0),
          canvas(0),
          viewConverter(vc),
          gridDrawer(0),
          currentExposure( NOT_DEFAULT_EXPOSURE )
        {
        }

    KisPrescaledProjectionSP prescaledProjection;
    KoToolProxy * toolProxy;
    KisCanvas2 * canvas;
    const KoViewConverter * viewConverter;
    QBrush checkBrush;
    QPoint documentOffset;
    KisGridDrawer* gridDrawer;
    double currentExposure;
};

KisQPainterCanvas::KisQPainterCanvas(KisCanvas2 * canvas, QWidget * parent)
    : QWidget( parent )
    , m_d( new Private( canvas->viewConverter() ) )
{
    // XXX: Reset pattern size and color when the properties change!

    KisConfig cfg;

    m_d->canvas =  canvas;
    m_d->gridDrawer = new QPainterGridDrawer(canvas->view()->document(), canvas->viewConverter());
    m_d->toolProxy = canvas->toolProxy();
    setAutoFillBackground(true);
    //setAttribute( Qt::WA_OpaquePaintEvent );
    m_d->checkBrush = QBrush(checkImage(cfg.checkSize()));
    setAcceptDrops( true );
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled, true);
}

KisQPainterCanvas::~KisQPainterCanvas()
{
    delete m_d->gridDrawer;
    delete m_d;
}

void KisQPainterCanvas::setPrescaledProjection( KisPrescaledProjectionSP prescaledProjection )
{
    m_d->prescaledProjection = prescaledProjection;
}

void KisQPainterCanvas::paintEvent( QPaintEvent * ev )
{
    QPixmap pm( ev->rect().size() );

    KisConfig cfg;

    kDebug(41010) <<"paintEvent: rect" << ev->rect() <<", doc offset:" << m_d->documentOffset;
    KisImageSP img = m_d->canvas->image();
    if (img == 0) return;

    if (img->colorSpace()->hasHighDynamicRange() &&
        (m_d->currentExposure != m_d->canvas->view()->resourceProvider()->HDRExposure())) {
        // XXX: If we had a dirty region we could just recomposite areas as
        // they become visible.
        // YYY: As soon as we start using
        // KisProjection::setRegionOfInterest(), only the visible
        // areas will be updated. (Boud)
        QApplication::setOverrideCursor(Qt::WaitCursor);
        m_d->canvas->updateCanvasProjection(img->bounds());
        QApplication::restoreOverrideCursor();
        m_d->currentExposure = m_d->canvas->view()->resourceProvider()->HDRExposure();
    }

    QTime t;

    setAutoFillBackground(false);

    QPainter gc( &pm );

    gc.translate( -ev->rect().topLeft() );

    gc.setCompositionMode( QPainter::CompositionMode_Source );

    t.start();
    // Don't draw the checks if we draw a cached pixmap, because we
    // need alpha transparency for checks. The precached pixmap
    // already should contain checks.
    if ( !cfg.noXRender() ) {

        if ( cfg.scrollCheckers() ) {

            QRect fillRect = ev->rect();

            if (m_d->documentOffset.x() > 0) {
                fillRect.adjust(0, 0, m_d->documentOffset.x(), 0);
            } else {
                fillRect.adjust(m_d->documentOffset.x(), 0, 0, 0);
            }
            if (m_d->documentOffset.y() > 0) {
                fillRect.adjust(0, 0, 0, m_d->documentOffset.y());
            } else {
                fillRect.adjust(0, m_d->documentOffset.y(), 0, 0);
            }
            gc.save();
            gc.translate(-m_d->documentOffset );
            kDebug(41010) << "qpainter canvas fillRect: " << fillRect;
            gc.fillRect( fillRect, m_d->checkBrush );
            gc.restore();
        }
        else {
            // Checks
            kDebug(41010) << "qpainter canvas fillRect: " << ev->rect();
            gc.fillRect(ev->rect(), m_d->checkBrush );
        }
        kDebug(41010) <<"Painting checks:" << t.elapsed();
    }
    t.restart();
    gc.setCompositionMode( QPainter::CompositionMode_SourceOver );

    if ( cfg.noXRender() ) {
        gc.drawPixmap( ev->rect(), m_d->prescaledProjection->prescaledPixmap(), ev->rect() );
    }
    else {
        gc.drawImage( ev->rect(), m_d->prescaledProjection->prescaledQImage(), ev->rect() );
    }
    kDebug(41010) <<"Drawing image:" << t.elapsed();

#ifdef DEBUG_REPAINT
    QColor color = QColor(random()%255, random()%255, random()%255, 150);
    gc.fillRect(ev->rect(), color);
#endif

    // ask the current layer to paint its selection (and potentially
    // other things, like wetness and active-layer outline

    // XXX: make settable
    bool drawAnts = true;
    bool drawGrids = true;
    bool drawTools = true;

    drawDecorations(gc, drawAnts, drawGrids, drawTools, m_d->documentOffset, ev->rect(), m_d->canvas, m_d->gridDrawer );

    gc.end();

    QPainter gc2( this );
    t.restart();
    gc2.drawPixmap( ev->rect().topLeft(), pm );
    kDebug(41010 ) <<"Drawing pixmap on widget:" << t.elapsed();
}

void KisQPainterCanvas::mouseMoveEvent(QMouseEvent *e) {
    m_d->toolProxy->mouseMoveEvent( e, m_d->viewConverter->viewToDocument(e->pos() + m_d->documentOffset ) );
}

void KisQPainterCanvas::mousePressEvent(QMouseEvent *e) {
    m_d->toolProxy->mousePressEvent( e, m_d->viewConverter->viewToDocument(e->pos() + m_d->documentOffset ) );
    if(e->button() == Qt::RightButton) {
        m_d->canvas->view()->unplugActionList( "flake_tool_actions" );
        m_d->canvas->view()->plugActionList( "flake_tool_actions",
                                             m_d->toolProxy->popupActionList() );
        QMenu *menu = dynamic_cast<QMenu*> (m_d->canvas->view()->factory()->container("image_popup", m_d->canvas->view()));
        if(menu)
            menu->exec( e->globalPos() );
    }
}

void KisQPainterCanvas::mouseReleaseEvent(QMouseEvent *e) {
    m_d->toolProxy->mouseReleaseEvent( e, m_d->viewConverter->viewToDocument(e->pos() + m_d->documentOffset ) );
}

void KisQPainterCanvas::mouseDoubleClickEvent(QMouseEvent *e) {
    m_d->toolProxy->mouseDoubleClickEvent( e, m_d->viewConverter->viewToDocument(e->pos() + m_d->documentOffset ) );
}

void KisQPainterCanvas::keyPressEvent( QKeyEvent *e ) {
    m_d->toolProxy->keyPressEvent(e);
}

void KisQPainterCanvas::keyReleaseEvent (QKeyEvent *e) {
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

void KisQPainterCanvas::tabletEvent( QTabletEvent *e )
{
    kDebug(41010) <<"tablet event:" << e->pressure();
    QPointF pos = e->pos() + (e->hiResGlobalPos() - e->globalPos());
    pos += m_d->documentOffset;
    m_d->toolProxy->tabletEvent( e, m_d->viewConverter->viewToDocument( pos ) );
}

void KisQPainterCanvas::wheelEvent( QWheelEvent *e )
{
    m_d->toolProxy->wheelEvent( e, m_d->viewConverter->viewToDocument( e->pos() + m_d->documentOffset ) );
}

bool KisQPainterCanvas::event (QEvent *event) {
    // we should forward tabs, and let tools decide if they should be used or ignored.
    // if the tool ignores it, it will move focus.
    if(event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*> (event);
        if(keyEvent->key() == Qt::Key_Backtab)
            return true;
        if(keyEvent->key() == Qt::Key_Tab && event->type() == QEvent::KeyPress) {
            // we loose key-release events, which I think is not an issue.
            keyPressEvent(keyEvent);
            return true;
        }
    }
    return QWidget::event(event);
}

KoToolProxy * KisQPainterCanvas::toolProxy()
{
    return m_d->toolProxy;
}

void KisQPainterCanvas::documentOffsetMoved( QPoint pt )
{
    kDebug(41010) << "KisQPainterCanvas::documentOffsetMoved " << pt;
    m_d->documentOffset = pt;
    m_d->prescaledProjection->documentOffsetMoved( pt );
    update();
}

void KisQPainterCanvas::resizeEvent( QResizeEvent *e )
{
    kDebug(41010) << "KisQPainterCanvas::resizeEvent : " << e->size();
    m_d->prescaledProjection->resizePrescaledImage( e->size() );
}

#include "kis_qpainter_canvas.moc"
