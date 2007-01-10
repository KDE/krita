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
#include <QDebug>

#include <kdebug.h>

#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <KoToolManager.h>
#include <KoToolProxy.h>

#include <kis_image.h>
#include <kis_layer.h>
#include <kis_meta_registry.h>

#include "kis_config.h"
#include "kis_canvas2.h"
#include "kis_resource_provider.h"
#include "kis_view2.h"

#define PATTERN_WIDTH 256
#define PATTERN_HEIGHT 256


class KisQPainterCanvas::Private {
public:
    KoToolProxy * toolProxy;
    KisCanvas2 * canvas;
    KoViewConverter * viewConverter;
    QImage * checkTexture;
    QBrush * checkBrush;
    QImage * displayCache;
};

KisQPainterCanvas::KisQPainterCanvas(KisCanvas2 * canvas, QWidget * parent)
    : QWidget( parent )

{
    m_d = new Private();
    m_d->canvas =  canvas;
    m_d->viewConverter = canvas->viewConverter();
    m_d->toolProxy = KoToolManager::instance()->createToolProxy(m_d->canvas);
    m_d->displayCache = 0;
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);
    m_d->checkTexture = new QImage(PATTERN_WIDTH, PATTERN_HEIGHT, QImage::Format_RGB32);

    for (int y = 0; y < PATTERN_HEIGHT; y++)
    {
        for (int x = 0; x < PATTERN_WIDTH; x++)
        {
            // XXX: make size of checks configurable
            quint8 v = 128 + 63 * ((x / 16 + y / 16) % 2);
            m_d->checkTexture->setPixel(x, y, qRgb(v, v, v));
        }
    }

    m_d->checkBrush = new QBrush( *m_d->checkTexture );
}


KisQPainterCanvas::~KisQPainterCanvas()
{
    delete m_d->checkTexture;
    delete m_d->checkBrush;
    delete m_d->displayCache;
    delete m_d;
}

#define EPSILON 1e-6

void KisQPainterCanvas::paintEvent( QPaintEvent * ev )
{
    QImage canvasPixmap = m_d->canvas->canvasCache();

    KisImageSP img = m_d->canvas->image();
    if (img == 0) return;

    QTime t;

    setAutoFillBackground(false);

    QRegion paintRegion = ev->region();
    QPainter gc( this );
    gc.setRenderHint(QPainter::SmoothPixmapTransform, false);
    gc.setRenderHint(QPainter::Antialiasing, false);

    // Then draw the checks in the rects that are inside the image
    // and which we need to repaint. We must paint all checks because we
    // don't know where our image is transparent. In the same loop, ask
    // the image to paint itself. (And later, the selections and so
    // on.)

    QVector<QRect> repaintRects = paintRegion.rects();

    kDebug(41010) << "painting on " << repaintRects.count() << " rects\n";

    QVector<QRect>::iterator it = repaintRects.begin();
    QVector<QRect>::iterator end = repaintRects.end();

    while (it != end) {

        kDebug(41010) << "Starting with checkers on rect " << (*it) << endl;
        t.start();
        // Checks
        gc.fillRect((*it), *m_d->checkBrush );

        qDebug( "Painting checks: %d", t.elapsed() );
        t.restart();

        double sx, sy;
        m_d->viewConverter->zoom(&sx, &sy);
        double pppx,pppy;
        pppx = img->xRes();
        pppy = img->yRes();
        QRectF imageRect = m_d->viewConverter->viewToDocument(*it);
        imageRect.adjust(-1, -1, 1, 1);
        imageRect.setCoords(imageRect.left() * pppx, imageRect.top() * pppy,
                            imageRect.right() * pppx, imageRect.bottom() * pppy);
        QRect rc = imageRect.toRect();


#if 1
        // XXX: Do I need to do stuff with EPSILON here because I'm
        // comparing doubles?
        if ( sx / pppx != 1.0 && sy / pppy != 1.0 ) {

            // XXX scale the canvaspixmap ourselves onto the top-left
            // region of the canvascache, and draw that without scaling
            // onto the widget painter.
            gc.drawImage( rc.x(), rc.y(), *m_d->displayCache, 0, 0, rc.width(), rc.height() );
        }
        else {
            // Draw the canvas pixmap unscaled at the right place
            gc.drawImage( rc.x(), rc.y(), canvasPixmap, rc.x(), rc.y(), rc.width(), rc.height() );

        }
#else
        gc.setWorldMatrixEnabled( true );
        kDebug(41010) << "scale: " << sx / pppx << ", " << sy / pppy << endl;
        gc.scale( sx / pppx, sy / pppy );

        QRectF imageRect = m_d->viewConverter->viewToDocument(*it);
        imageRect.adjust(-1, -1, 1, 1);
        imageRect.setCoords(imageRect.left() * pppx, imageRect.top() * pppy,
                            imageRect.right() * pppx, imageRect.bottom() * pppy);
        QRect rc = imageRect.toRect();

        gc.drawImage( rc.x(), rc.y(), canvasPixmap, rc.x(), rc.y(), rc.width(), rc.height() );
#endif
        qDebug( "painting image: %d",  t.elapsed() );
        t.restart();



#if 0 // Old code that uses KIsImage to scale down

        if(sx < 1.0 +EPSILON && sy < 1.0 +EPSILON) {
            // We are scaling pixels down (birds eye) so adjust for
            // display profile AFTER scaling the pixels
            kDebug(41010) << "Starting to paint scaled down\n";
            // Image
            QRectF imageRect = m_d->viewConverter->viewToDocument(*it);
            imageRect.adjust(-5,-5,5,5);
            imageRect.setCoords(imageRect.left() * pppx, imageRect.top() * pppy,
                                imageRect.right() * pppx, imageRect.bottom() * pppy);

            QImage image = img->convertToQImage(imageRect.toRect(), sx / pppx, sy / pppy,
                                                m_d->canvas->monitorProfile(), m_d->canvas->view()->resourceProvider()->HDRExposure());

            gc.drawImage(imageRect.topLeft(), image, image.rect());

            qDebug("Painting scaled down rects %d", t.elapsed() );
            t.restart();
        }
        else {
            kDebug(41010) << "Starting to paint magnified\n";
            // We are scaling pixels up (magnified look) so adjust for display profile before scaling the pixels
            gc.setWorldMatrixEnabled(true);
            gc.scale(sx/pppx, sy/pppy);

            // Image
            QRectF imageRect = m_d->viewConverter->viewToDocument(*it);
            imageRect.adjust(-5,-5,5,5);
            imageRect.setCoords(imageRect.left() * pppx, imageRect.top() * pppy,
                                imageRect.right() * pppx, imageRect.bottom() * pppy);
            // XXX: Added explict cast
            img->renderToPainter(static_cast<qint32>(imageRect.x()),
                                 static_cast<qint32>(imageRect.y()),
                                 static_cast<qint32>(imageRect.x()),
                                 static_cast<qint32>(imageRect.y()),
                                 static_cast<qint32>(imageRect.width()),
                                 static_cast<qint32>(imageRect.height()),
                                 gc,
                                 m_d->canvas->monitorProfile(),
                                 m_d->canvas->view()->resourceProvider()->HDRExposure());
            qDebug( "Done painting scaled up pixels %d", t.elapsed() );
            t.restart();
        }
#endif
        ++it;
    }

#if 0
    // ask the current layer to paint its selection (and potentially
    // other things, like wetness and active-layer outline
    KisLayerSP currentLayer = img->activeLayer();
    QVector<QRect>layerRects = QRegion(currentLayer->extent().translate(xoffset, yoffset))
                                .intersected(paintRegions);

    it = outsideRects.begin();
    end = outsideRects.end();
    while (it != end) {
            currentLayer->renderDecorationsToPainter((*it).x() - xoffset,
                                                     (*it).y() - yoffset,
                                                     (*it).x(), (*it).y(),
                                                     (*it).width(), (*it).height(),
                                                     gc);
    }
#endif
    // ask the guides, grids, etc to paint themselves

    // Give the tool a chance to paint its stuff
    kDebug(41010) << "Tool starts painting\n";
    m_d->toolProxy->paint(gc, *m_d->viewConverter );

    qDebug( "Done painting tool stuff %d", t.elapsed() );

    gc.end();
}


void KisQPainterCanvas::mouseMoveEvent(QMouseEvent *e) {
    m_d->toolProxy->mouseMoveEvent( e, m_d->viewConverter->viewToDocument(e->pos()) );
}

void KisQPainterCanvas::mousePressEvent(QMouseEvent *e) {
    m_d->toolProxy->mousePressEvent( e, m_d->viewConverter->viewToDocument(e->pos()) );
}

void KisQPainterCanvas::mouseReleaseEvent(QMouseEvent *e) {
    m_d->toolProxy->mouseReleaseEvent( e, m_d->viewConverter->viewToDocument(e->pos()) );
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
    m_d->toolProxy->tabletEvent( e, m_d->viewConverter->viewToDocument(  e->pos() ) );
}

void KisQPainterCanvas::wheelEvent( QWheelEvent *e )
{
    m_d->toolProxy->wheelEvent( e, m_d->viewConverter->viewToDocument( e->pos()  ) );
}

KoToolProxy * KisQPainterCanvas::toolProxy()
{
    return m_d->toolProxy;
}

void KisQPainterCanvas::parentSizeChanged(const QSize & size )
{
    if ( size.width() > m_d->displayCache->size().width() ||
         size.height() > m_d->displayCache->size().height() ) {
        kDebug() << "KisQPainterCanvas::parentSizeChanged " << size << endl;
        m_d->displayCache = new QImage(size, QImage::Format_RGB32);
    }
}

#include "kis_qpainter_canvas.moc"
