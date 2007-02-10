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

#define PATTERN_WIDTH 32
#define PATTERN_HEIGHT 32

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
    QPixmap displayCache;
};

KisQPainterCanvas::KisQPainterCanvas(KisCanvas2 * canvas, QWidget * parent)
    : QWidget( parent )

{
    m_d = new Private();
    m_d->canvas =  canvas;
    m_d->viewConverter = canvas->viewConverter();
    m_d->toolProxy = KoToolManager::instance()->createToolProxy(m_d->canvas);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);

    QPixmap tile(PATTERN_WIDTH * 2, PATTERN_HEIGHT * 2);
    tile.fill(Qt::white);
    QPainter pt(&tile);
    QColor color(220, 220, 220);
    pt.fillRect(0, 0, PATTERN_WIDTH, PATTERN_HEIGHT, color);
    pt.fillRect(PATTERN_WIDTH, PATTERN_HEIGHT, PATTERN_WIDTH, PATTERN_HEIGHT, color);
    pt.end();
    QBrush b(tile);

    m_d->checkBrush = b;
}


KisQPainterCanvas::~KisQPainterCanvas()
{
    delete m_d;
}

#define EPSILON 1e-6

void KisQPainterCanvas::paintEvent( QPaintEvent * ev )
{
    QRect updateRect = ev->rect();

//     kDebug(41010) << "Paint event: " << updateRect << endl;

    const QImage canvasImage = m_d->canvas->canvasCache();
#if 0
    if ( m_d->displayCache.width() < updateRect.width() ||
         m_d->displayCache.height() < updateRect.height() )
    {
        QSize size = updateRect.size();
        size.setWidth( size.width() + 64 );
        size.setHeight( size.height() + 64 );

//         kDebug(41010) << "Oops, display cache too small\n";
        m_d->displayCache = QPixmap( size );
        QPainter p( &m_d->displayCache );
        p.fillRect( 0, 0, m_d->displayCache.width(), m_d->displayCache.height(), m_d->checkBrush );
        p.end();

    }
#endif
    KisImageSP img = m_d->canvas->image();
    if (img == 0) return;

    QTime t;

    setAutoFillBackground(false);

    QPainter gc( this );

    double sx, sy;
    m_d->viewConverter->zoom(&sx, &sy);

    t.start();
    // Checks
    gc.fillRect(updateRect, m_d->checkBrush );
    //gc.drawPixmap( - (updateRect.x() % PATTERN_WIDTH ),
    //               -( updateRect.y() % PATTERN_HEIGHT ),
    //               m_d->displayCache );
//     kDebug(41010) << "Painting checks:" << t.elapsed() << endl;
    t.restart();

    double pppx,pppy;
    pppx = img->xRes();
    pppy = img->yRes();

    // Go from the widget coordinates to points
    QRectF imageRect = m_d->viewConverter->viewToDocument(updateRect);

    // Go from points to pixels
    imageRect.setCoords(imageRect.left() * pppx, imageRect.top() * pppy,
                        imageRect.right() * pppx, imageRect.bottom() * pppy);

    // Because when too small, the scaling will make the update disappear
    imageRect.adjust(-5, -5, 5, 5);

    // Don't go outside the image and convert to whole pixels
    QRect rc = toAlignedRect(imageRect.intersected( canvasImage.rect() ));

    // Compute the scale factors
    double scaleX = sx / pppx;
    double scaleY = sy / pppy;

    QPoint dstTopLeft = QPointF(rc.x() * scaleX, rc.y() * scaleY).toPoint();

    // Pixel-for-pixel mode
    if ( scaleX == 1.0 && scaleY == 1.0 ) {
//         kDebug() << "Pixel for pixel!\n";
        gc.drawImage( dstTopLeft.x(), dstTopLeft.y(), canvasImage, rc.x(), rc.y(), rc.width(), rc.height() );
    }
    else {
        QSize sz = QSize( ( int )( rc.width() * ( sx / pppx ) ), ( int )( rc.height() * ( sy / pppy ) ));
        t.restart();
        QImage croppedImage = canvasImage.copy( rc ); // This is way
                                                      // faster than
                                                      // in Qt 3.x. No
                                                      // need for
                                                      // GwenView's
                                                      // croppeqimage
                                                      // class anymore.
//         kDebug(41010) << "Copying subrect:" << t.elapsed() << endl;
        t.restart();

        QImage scaledImage;

        if ( scaleX > 1.0 && scaleY > 1.0 ) {
            scaledImage = ImageUtils::sampleImage( croppedImage, sz.width(), sz.height() );
        }
        else {
            scaledImage = ImageUtils::scale( croppedImage, sz.width(), sz.height() );
        }

//         kDebug(41010) << "Scaling subimage: " << t.elapsed() << endl;
        t.restart();
        gc.drawImage( dstTopLeft.x(), dstTopLeft.y(), scaledImage, 0, 0, sz.width(), sz.height() );
    }
//     kDebug(41010 ) << "painting image: " <<  t.elapsed() << endl;

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
    gc.setRenderHint( QPainter::Antialiasing );
    gc.setRenderHint( QPainter::SmoothPixmapTransform );
    m_d->toolProxy->paint(gc, *m_d->viewConverter );
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
//     kDebug(41010) << "tablet event: " << e->pressure() << endl;
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
    Q_UNUSED( size );
#if 0
    if (m_d->displayCache.isNull() ||
        ( size.width() > m_d->displayCache.width() ||
          size.height() > m_d->displayCache.height())
        )
    {
//         kDebug() << "KisQPainterCanvas::parentSizeChanged " << size << endl;
        m_d->displayCache = QPixmap( size.width() + 64, size.height() + 64 );
        QPainter p( &m_d->displayCache );
        p.fillRect( 0, 0, m_d->displayCache.width(), m_d->displayCache.height(), m_d->checkBrush );
        p.end();
    }
#endif
}

#include "kis_qpainter_canvas.moc"
