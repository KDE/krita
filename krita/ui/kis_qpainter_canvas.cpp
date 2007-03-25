/* This file is part of the KDE project
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

#include <QPaintEvent>
#include <QRect>
#include <QPainter>
#include <QImage>
#include <QBrush>
#include <QColor>
#include <QString>
#include <QTime>
#include <QPixmap>

#include <kdebug.h>

#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <KoZoomHandler.h>
#include <KoToolManager.h>
#include <KoToolProxy.h>

#include <kis_image.h>
#include <kis_layer.h>
#include <kis_meta_registry.h>

#include "kis_config.h"
#include "kis_canvas2.h"
#include "kis_resource_provider.h"
#include "kis_view2.h"
#include "scale.h"
#include "kis_doc2.h"
#include "kis_grid_drawer.h"

#define PATTERN_WIDTH 32
#define PATTERN_HEIGHT 32

//#define DEBUG_REPAINT

namespace {
// XXX: Remove this with Qt 4.3
    static QRect toAlignedRect(QRectF rc)
    {
        int xmin = int(floor(rc.x()));
        int xmax = int(ceil(rc.x() + rc.width()));
        int ymin = int(floor(rc.y()));
        int ymax = int(ceil(rc.y() + rc.height()));
        return QRect(xmin, ymin, xmax - xmin, ymax - ymin);
    }
}


class KisQPainterCanvas::Private {
public:
    KoToolProxy * toolProxy;
    KisCanvas2 * canvas;
    KoViewConverter * viewConverter;
    QBrush checkBrush;
    QImage prescaledImage;
    QPoint documentOffset;
    QPainterGridDrawer* gridDrawer;
};

KisQPainterCanvas::KisQPainterCanvas(KisCanvas2 * canvas, QWidget * parent)
    : QWidget( parent )

{
    // XXX: Reset pattern size and color when the properties change!

    KisConfig cfg;

    m_d = new Private();
    m_d->canvas =  canvas;
    m_d->viewConverter = canvas->viewConverter();
    m_d->gridDrawer = new QPainterGridDrawer(canvas->view()->document(), canvas->viewConverter());
    m_d->toolProxy = KoToolManager::instance()->createToolProxy(m_d->canvas);
    setAutoFillBackground(true);
    //setAttribute( Qt::WA_OpaquePaintEvent );
    QPixmap tile(cfg.checkSize() * 2, cfg.checkSize() * 2);
    tile.fill(Qt::white);
    QPainter pt(&tile);
    pt.fillRect(0, 0, cfg.checkSize(), cfg.checkSize(), cfg.checkersColor());
    pt.fillRect(cfg.checkSize(), cfg.checkSize(), cfg.checkSize(), cfg.checkSize(), cfg.checkersColor());
    pt.end();
    m_d->checkBrush = QBrush(tile);
    setAcceptDrops( true );
    setFocusPolicy(Qt::StrongFocus);
}

KisQPainterCanvas::~KisQPainterCanvas()
{
    delete m_d;
}

#define EPSILON 1e-6

void KisQPainterCanvas::paintEvent( QPaintEvent * ev )
{
    KisConfig cfg;

    kDebug(41010) << "paintEvent: rect " << ev->rect() << ", doc offset: " << m_d->documentOffset << endl;
    KisImageSP img = m_d->canvas->image();
    if (img == 0) return;

    QTime t;

    setAutoFillBackground(false);

    QPainter gc( this );

    double sx, sy;
    m_d->viewConverter->zoom(&sx, &sy);

    t.start();
    if ( !cfg.scrollCheckers() ) {
        // Checks

        gc.fillRect(ev->rect(), m_d->checkBrush );
        kDebug(41010) << "Painting checks:" << t.elapsed() << endl;

        t.restart();
    }
    gc.drawImage( 0, 0, m_d->prescaledImage );
    kDebug(41010) << "Drawing image:" << t.elapsed() << endl;

#ifdef DEBUG_REPAINT
    QColor color = QColor(random()%255, random()%255, random()%255, 150);
    gc.fillRect(ev->rect(), color);
#endif

    // ask the current layer to paint its selection (and potentially
    // other things, like wetness and active-layer outline


    // Setup the painter to take care of the offset; all that the
    // classes that do painting need to keep track of is resolution
    gc.setRenderHint( QPainter::Antialiasing );
    gc.setRenderHint( QPainter::SmoothPixmapTransform );
    gc.translate( QPoint( -m_d->documentOffset.x(), -m_d->documentOffset.y() ) );

    // ask the guides, grids, etc to paint themselves
    t.restart();
    m_d->gridDrawer->draw(&gc, m_d->viewConverter->viewToDocument(ev->rect()));
    kDebug(41010) << "Drawing grid:" << t.elapsed() << endl;

    t.restart();
    // Give the tool a chance to paint its stuff
    m_d->toolProxy->paint(gc, *m_d->viewConverter );
    kDebug(41010) << "Drawing tools:" << t.elapsed() << endl;
}

void KisQPainterCanvas::mouseMoveEvent(QMouseEvent *e) {
    m_d->toolProxy->mouseMoveEvent( e, m_d->viewConverter->viewToDocument(e->pos() + m_d->documentOffset ) );
}

void KisQPainterCanvas::mousePressEvent(QMouseEvent *e) {
    m_d->toolProxy->mousePressEvent( e, m_d->viewConverter->viewToDocument(e->pos() + m_d->documentOffset ) );
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

void KisQPainterCanvas::tabletEvent( QTabletEvent *e )
{
    kDebug(41010) << "tablet event: " << e->pressure() << endl;
    m_d->toolProxy->tabletEvent( e, m_d->viewConverter->viewToDocument(  e->pos() + m_d->documentOffset ) );
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
    qint32 width = m_d->prescaledImage.width();
    qint32 height = m_d->prescaledImage.height();

    QRegion exposedRegion = QRect(0, 0, width, height);

    qint32 oldCanvasXOffset = m_d->documentOffset.x();
    qint32 oldCanvasYOffset = m_d->documentOffset.y();

    m_d->documentOffset = pt;

    QImage img = QImage( width, height, QImage::Format_ARGB32 );
    QPainter gc( &img );

    if (!m_d->prescaledImage.isNull()) {

        if (oldCanvasXOffset != m_d->documentOffset.x() || oldCanvasYOffset != m_d->documentOffset.y()) {

            qint32 deltaX = m_d->documentOffset.x() - oldCanvasXOffset;
            qint32 deltaY = m_d->documentOffset.y() - oldCanvasYOffset;

            gc.drawImage( -deltaX, -deltaY, m_d->prescaledImage );
            exposedRegion -= QRegion(QRect(-deltaX, -deltaY, width - deltaX, height - deltaY));
        }
    }


    if (!m_d->prescaledImage.isNull() && !exposedRegion.isEmpty()) {

        QVector<QRect> rects = exposedRegion.rects();

        for (int i = 0; i < rects.count(); i++) {
            QRect r = rects[i];
            drawScaledImage( r, gc);
        }
    }
    m_d->prescaledImage = img;
    update();
}


void KisQPainterCanvas::drawScaledImage( const QRect & r, QPainter &gc )
{
    KisImageSP img = m_d->canvas->image();
    if (img == 0) return;
    QRect rc = r;

    const QImage canvasImage = m_d->canvas->canvasCache();

    double sx, sy;
    m_d->viewConverter->zoom(&sx, &sy);

    // Compute the scale factors
    double scaleX = sx / img->xRes();
    double scaleY = sy / img->yRes();

    // comput how large a fully scaled image is
    QSize dstSize = QSize(int(canvasImage.width() * scaleX ), int( canvasImage.height() * scaleY));

    // Don't go outside the image (will crash the sampleImage method below)
    QRect drawRect = rc.translated( m_d->documentOffset).intersected(QRect(QPoint(),dstSize));

    // Pixel-for-pixel mode
    if ( scaleX == 1.0 && scaleY == 1.0 ) {
        gc.drawImage( rc.topLeft(), canvasImage.copy( drawRect ) );
    }
    else {
        if ( scaleX > 2.0 && scaleY > 2.0 ) {
            gc.drawImage( rc.topLeft(), ImageUtils::sampleImage(canvasImage,
                     dstSize.width(), dstSize.height(), drawRect));
        }
        else {
            // Go from the widget coordinates to points
            QRectF imageRect = m_d->viewConverter->viewToDocument( rc.translated( m_d->documentOffset ) );

            double pppx,pppy;
            pppx = img->xRes();
            pppy = img->yRes();

            imageRect.setCoords(imageRect.left() * pppx, imageRect.top() * pppy,
                        imageRect.right() * pppx, imageRect.bottom() * pppy);

            // Don't go outside the image and convert to whole pixels
            QRect alignedRect = toAlignedRect(imageRect.intersected( canvasImage.rect() ));

            QSize sz = QSize( ( int )( alignedRect.width() * scaleX ), ( int )( alignedRect.height() * scaleY ));
            QImage croppedImage = canvasImage.copy( alignedRect );
            gc.drawImage( rc.topLeft(), ImageUtils::scale( croppedImage, sz.width(), sz.height() ));
        }
    }
}

void KisQPainterCanvas::resizeEvent( QResizeEvent *e )
{
    QTime t;
    t.start();


    QSize newSize = e->size();
    QSize oldSize = m_d->prescaledImage.size();

    QImage img = QImage(e->size(), QImage::Format_ARGB32);
    QPainter gc( &img );

    gc.drawImage( 0, 0, m_d->prescaledImage, 0, 0, m_d->prescaledImage.width(), m_d->prescaledImage.height() );

    if ( newSize.width() > oldSize.width() || newSize.height() > oldSize.height() ) {

        QRect right( oldSize.width(), 0, newSize.width() - oldSize.width(), newSize.height() );
        if ( right.width() > 0 ) {
            drawScaledImage( right, gc);
        }
        // Subtract the right hand overlap part (right.width() from
        // the bottom band so we don't scale the same area twice.
        QRect bottom( 0, oldSize.height(), newSize.width() - right.width(), newSize.height() - oldSize.height() );
        if ( bottom.height() > 0 ) {
            drawScaledImage( bottom, gc);
        }

    }
    m_d->prescaledImage = img;

    kDebug(41010) << "Resize event:" << t.elapsed() << endl;
}

void KisQPainterCanvas::preScale()
{
    KisConfig cfg;
    // Thread this!
    QTime t;
    t.start();
    m_d->prescaledImage = QImage( size(), QImage::Format_ARGB32);

    QPainter gc( &m_d->prescaledImage );
    if ( cfg.scrollCheckers() ) {
        gc.fillRect(m_d->prescaledImage.rect(), m_d->checkBrush );
    }

    drawScaledImage( QRect( QPoint( 0, 0 ), size() ), gc);
    kDebug(41010) << "preScale():" << t.elapsed() << endl;

}

void KisQPainterCanvas::preScale( const QRect & rc )
{
    if ( !rc.isEmpty() ) {
        QTime t;
        t.start();
        QPainter gc( &m_d->prescaledImage );
        drawScaledImage( rc, gc);
        kDebug(41010) << "Prescaling took " << t.elapsed() << endl;
    }
}

#include "kis_qpainter_canvas.moc"
