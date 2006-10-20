/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
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
    , m_monitorProfile( 0 )
    , m_checkTexture( 0 )
    , m_checkBrush( 0 )

{
    m_toolProxy = KoToolManager::instance()->toolProxy();

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

    m_monitorProfile = KoColorProfile::getScreenProfile();

    if (m_monitorProfile == 0) {
        KisConfig cfg;
        QString monitorProfileName = cfg.monitorProfile();
        m_monitorProfile = KisMetaRegistry::instance()->csRegistry()->profileByName(monitorProfileName);
    }

}


KisQPainterCanvas::~KisQPainterCanvas()
{
    delete m_checkTexture;
    delete m_checkBrush;
}

void KisQPainterCanvas::paintEvent( QPaintEvent * ev )
{
    KisImageSP img = m_canvas->image();
    if (img == 0) return;

    setAutoFillBackground(false);
    QRegion paintRegion = ev->region();
    QPainter gc( this );

    QRegion imageRegion = m_canvas->image()->extent();

    // The widget origin is 0,0 -- the image may well have a negative x, y, so we need to move all rects.
    qint32 xoffset = -imageRegion.boundingRect().x();
    qint32 yoffset = -imageRegion.boundingRect().y();

    imageRegion.translate(xoffset, yoffset);

    // Only the part that intersects with the bit we want to paint is interesting
    QRect imageRect = QRect(xoffset, yoffset, img->width(), img->height());

    // If we're in the border area, draw the border.
    // The border region is the set of rects around the image where there may be
    // layer data, but that is outside the user-defined dimensions of the image.
    // But we are only interested in the region that intersects with the region
    // we need to repaint.
    QRegion borderRegion = QRegion(imageRegion.boundingRect())
        .subtracted(QRegion(imageRect))
        .intersected(paintRegion);

    QVector<QRect> borderRects = borderRegion.rects();

    QVector<QRect>::iterator it = borderRects.begin();
    QVector<QRect>::iterator end = borderRects.end();

    while (it != end) {
        gc.fillRect((*it), palette().dark());
        ++it;
    }

    // Then draw the checks in the rects that are inside the the image
    // and which we need to repaint. We must paint all checks because we
    // don't know where our image is transparent. In the same loop, ask
    // the image to paint itself. (And later, the selections and so on.)
    if (paintRegion.intersects(imageRect)) {
        QVector<QRect> checkRects = paintRegion.intersected(QRegion(imageRect)).rects();

        QVector<QRect>::iterator it = checkRects.begin();
        QVector<QRect>::iterator end = checkRects.end();

        while (it != end) {
            // Checks
            gc.fillRect((*it), *m_checkBrush );
            // Image
            img->renderToPainter((*it).x() - xoffset,
                                 (*it).y() - yoffset,
                                 (*it).x(), (*it).y(),
                                 (*it).width(), (*it).height(), gc,
                                 m_monitorProfile,
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
    // Paint the bits of the layers that are outside the real image, with
    // a gray wash
    QVector<QRect>outsideRects = imageRegion
                                    .subtracted(QRegion(imageRect))
                                    .intersected(paintRegion).rects();

    it = outsideRects.begin();
    end = outsideRects.end();
    while (it != end) {
        img->renderToPainter((*it).x() - xoffset,
                             (*it).y() - yoffset,
                             (*it).x(), (*it).y(),
                             (*it).width(), (*it).height(), gc,
                             m_monitorProfile,
                             0);
        gc.save();
        gc.setOpacity(0.95);
        gc.fillRect(*it, palette().dark());
        gc.restore();
        ++it;
    }
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
    m_toolProxy->tabletEvent( e, m_viewConverter->viewToDocument(  e->pos() ) );
}

void KisQPainterCanvas::wheelEvent( QWheelEvent *e )
{
    m_toolProxy->wheelEvent( e, m_viewConverter->viewToDocument( e->pos()  ) );
}



#include "kis_qpainter_canvas.moc"
