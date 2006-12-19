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

#include <kdebug.h>

#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>

#include <KoToolProxy.h>
#include <KoToolManager.h>

#include <kis_meta_registry.h>
#include <kis_image.h>
#include <kis_layer.h>

#include "kis_config.h"
#include "kis_canvas2.h"

#define PATTERN_WIDTH 256
#define PATTERN_HEIGHT 256

KisQPainterCanvas::KisQPainterCanvas(KisCanvas2 * canvas, QWidget * parent)
    : QWidget( parent )
    , m_canvas( canvas )
    , m_viewConverter( canvas->viewConverter() )
    , m_checkTexture( 0 )
    , m_checkBrush( 0 )

{
    m_toolProxy = KoToolManager::instance()->createToolProxy(m_canvas);

    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);
    m_checkTexture = new QImage(PATTERN_WIDTH, PATTERN_HEIGHT, QImage::Format_RGB32);

    for (int y = 0; y < PATTERN_HEIGHT; y++)
    {
        for (int x = 0; x < PATTERN_WIDTH; x++)
        {
            // XXX: make size of checks configurable
            quint8 v = 128 + 63 * ((x / 16 + y / 16) % 2);
            m_checkTexture->setPixel(x, y, qRgb(v, v, v));
        }
    }

    m_checkBrush = new QBrush( *m_checkTexture );
}


KisQPainterCanvas::~KisQPainterCanvas()
{
    delete m_checkTexture;
    delete m_checkBrush;
}

#define EPSILON 1e-6

void KisQPainterCanvas::paintEvent( QPaintEvent * ev )
{
    KisImageSP img = m_canvas->image();
    if (img == 0) return;

    setAutoFillBackground(false);
    QRegion paintRegion = ev->region();
    QPainter gc( this );

    // Then draw the checks in the rects that are inside the image
    // and which we need to repaint. We must paint all checks because we
    // don't know where our image is transparent. In the same loop, ask
    // the image to paint itself. (And later, the selections and so on.)
    QVector<QRect> checkRects = paintRegion.rects();

    QVector<QRect>::iterator it = checkRects.begin();
    QVector<QRect>::iterator end = checkRects.end();

    while (it != end) {
        // Checks
        gc.fillRect((*it), *m_checkBrush );
        ++it;
    }

    double sx,sy;
    m_viewConverter->zoom(&sx, &sy);
    double pppx,pppy;
    pppx = img->xRes();
    pppy = img->yRes();

    if(sx < 1.0 +EPSILON && sy < 1.0 +EPSILON) {
        // We are scaling pixels down (birds eye) so adjust for display profile AFTER scaling the pixels
        it = checkRects.begin();
        while (it != end) {
            // Image
            QRectF imagerect = m_viewConverter->viewToDocument(*it);
            imagerect.adjust(-5,-5,5,5);
           imagerect.setCoords(imagerect.left()*pppx,imagerect.top()*pppy,
                            imagerect.right()*pppx,imagerect.bottom()*pppy);

            QImage image = img->convertToQImage(imagerect.toRect(), pppx/sx, pppy/sy, 
                        m_canvas->monitorProfile());

            gc.drawImage(imagerect.topLeft(), image, image.rect());
            ++it;
        }
    }
    else
    {
        // We are scaling pixels up (magnified look) so adjust for display profile before scaling the pixels
        gc.setWorldMatrixEnabled(true);
        gc.scale(sx/pppx, sy/pppy);

        it = checkRects.begin();
        while (it != end) {
           // Image
           QRectF imagerect = m_viewConverter->viewToDocument(*it);
           imagerect.adjust(-5,-5,5,5);
           imagerect.setCoords(imagerect.left()*pppx,imagerect.top()*pppy,
                            imagerect.right()*pppx,imagerect.bottom()*pppy);
           // XXX: Added explict cast
           img->renderToPainter(static_cast<qint32>(imagerect.x()),
                                static_cast<qint32>(imagerect.y()),
                                static_cast<qint32>(imagerect.x()), 
                                static_cast<qint32>(imagerect.y()),
                                static_cast<qint32>(imagerect.width()),
                                static_cast<qint32>(imagerect.height()),
                                gc,
                                m_canvas->monitorProfile(),
                                0);
            ++it;
        }
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
    m_toolProxy->paint(gc, *m_viewConverter );

    gc.end();
}


void KisQPainterCanvas::mouseMoveEvent(QMouseEvent *e) {
    m_toolProxy->mouseMoveEvent( e, m_viewConverter->viewToDocument(e->pos()) );
}

void KisQPainterCanvas::mousePressEvent(QMouseEvent *e) {
    m_toolProxy->mousePressEvent( e, m_viewConverter->viewToDocument(e->pos()) );
}

void KisQPainterCanvas::mouseReleaseEvent(QMouseEvent *e) {
    m_toolProxy->mouseReleaseEvent( e, m_viewConverter->viewToDocument(e->pos()) );
}

void KisQPainterCanvas::keyPressEvent( QKeyEvent *e ) {
    m_toolProxy->keyPressEvent(e);
}

void KisQPainterCanvas::keyReleaseEvent (QKeyEvent *e) {
    m_toolProxy->keyReleaseEvent(e);
}

void KisQPainterCanvas::tabletEvent( QTabletEvent *e )
{
    kDebug() << "tablet event: " << e->pressure() << endl;
    m_toolProxy->tabletEvent( e, m_viewConverter->viewToDocument(  e->pos() ) );
}

void KisQPainterCanvas::wheelEvent( QWheelEvent *e )
{
    m_toolProxy->wheelEvent( e, m_viewConverter->viewToDocument( e->pos()  ) );
}

#include "kis_qpainter_canvas.moc"
