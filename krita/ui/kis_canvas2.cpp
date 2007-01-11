/* This file is part of the KDE project
 *
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA..
 */
#include "kis_canvas2.h"

#include <QWidget>
#include <QTime>
#include <QPixmap>
#include <QLabel>

#include <kdebug.h>

#include <KoUnit.h>
#include <KoViewConverter.h>
#include <KoShapeManager.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KoDocument.h>

#include <kis_image.h>

#include "kis_resource_provider.h"
#include "kis_view2.h"
#include "kis_config.h"
#include "kis_abstract_canvas_widget.h"
#include "kis_qpainter_canvas.h"
#include "kis_opengl_canvas2.h"


class KisCanvas2::KisCanvas2Private {

public:

    KisCanvas2Private( KoCanvasBase * parent, KoViewConverter * viewConverter, KisView2 * view )
        : viewConverter( viewConverter )
        , view( view )
        , canvasWidget( 0 )
        , shapeManager( new KoShapeManager(parent) )
        , monitorProfile( 0 )
        {
        }

    ~KisCanvas2Private()
        {
            delete shapeManager;
            delete monitorProfile;
        }

    KoViewConverter * viewConverter;
    KisView2 * view;
    KisAbstractCanvasWidget * canvasWidget;
    KoShapeManager * shapeManager;
    KoColorProfile * monitorProfile;
    QImage canvasCache; // XXX: Make this a structure with
                        // QImage-based tiles, instead of one big
                        // QImage. Besides, QImage is only useful for
                        // the qpainter canvas, not the opengl canvas
                        // (that I still need to re-enable). But lets
                        // see whether this gives a speedup -- this is
                        // the same more or less as with the old
                        // opengl painter
};

KisCanvas2::KisCanvas2(KoViewConverter * viewConverter, KisCanvasType canvasType, KisView2 * view, KoShapeControllerBase * sc)
    : KoCanvasBase(sc)
{
    m_d = new KisCanvas2Private(this, viewConverter, view);

    switch( canvasType ) {
    case OPENGL:
#ifdef HAVE_OPENGL
        //setCanvasWidget( new KisOpenGLCanvas2( this, view ) );
        break;
#else
        kWarning() << "OpenGL requested while its not available";
        Q_ASSERT(false);
#endif
    case MITSHM:
    case QPAINTER:
    default:
        setCanvasWidget( new KisQPainterCanvas( this, view ) );
    }
}

void KisCanvas2::setCanvasWidget(QWidget * widget)
{
    KisAbstractCanvasWidget * tmp = dynamic_cast<KisAbstractCanvasWidget*>( widget );
    Q_ASSERT_X( tmp, "setCanvasWidget", "Cannot cast the widget to a KisAbstractCanvasWidget" );
    m_d->canvasWidget = tmp;
}

KisCanvas2::~KisCanvas2()
{
    delete m_d;
}

KisView2* KisCanvas2::view()
{
    return m_d->view;
}

void KisCanvas2::gridSize(double *horizontal, double *vertical) const
{
    Q_UNUSED( horizontal );
    Q_UNUSED( vertical );

}

bool KisCanvas2::snapToGrid() const
{
    return true;
}

void KisCanvas2::addCommand(QUndoCommand *command)
{
    m_d->view->koDocument()->addCommand( command );
}

KoShapeManager* KisCanvas2::shapeManager() const
{
    return m_d->shapeManager;
}


void KisCanvas2::updateCanvas(const QRectF& rc)
{
    kDebug(41010) << "KisCanvas2::updateCanvas(QRectF): " << rc << endl;

    // First convert from document coordinated to widget coordinates
    QRectF viewRect  = m_d->viewConverter->documentToView(rc);
    kDebug() << "KiSCanvas2::updateCanvas viewrect becomes: " << viewRect << endl;
    m_d->canvasWidget->widget()->update( viewRect.toRect() );
}


void KisCanvas2::updateCanvas(const QRegion & rc)
{
    kDebug(41007) << "KisCanvas2::updateCanvas(QRegion): " << rc << endl;

    // Loop through all rects in the region and call update() on the
    // canvas widget for all these rects after conversion: Qt will
    // then schedule a paint event for the rects in the region.
    QRegion widgetRegion;
    QVector<QRect> rcRects = rc.rects();

    QVector<QRect>::iterator it = rcRects.begin();
    QVector<QRect>::iterator end = rcRects.end();

    while ( it != end ) {
        it->adjust( -4, -4, 4, 4 );
        widgetRegion += QRegion( m_d->viewConverter->documentToView( *it ).toRect() );
        ++it;
    }
    m_d->canvasWidget->widget()->update( widgetRegion );
}

void KisCanvas2::updateCanvasProjection( const QRect & rc )
{
    kDebug(41010) << "KisCanvas2::updateCanvasProjection(QRect) " << rc << " onto cache " << m_d->canvasCache.size() << endl;
    QTime t;
    t.start();
    QPainter p( &m_d->canvasCache );
    p.drawImage( rc.x(), rc.y(),
                 image()->convertToQImage(rc.x(), rc.y(), rc.width(), rc.height(),
                                          m_d->monitorProfile,
                                          m_d->view->resourceProvider()->HDRExposure() )
                , 0, 0, rc.width(), rc.height() );
    kDebug(41010) << "Converting image part took " << t.elapsed() << " ms\n";

    double pppx,pppy;
    pppx = image()->xRes();
    pppy = image()->yRes();
    QRectF docRect;
    docRect.setCoords(rc.left() - 2 / pppx, rc.top() - 2 / pppy, rc.right() + 2 / pppx, rc.bottom() + 2 / pppy);
    QRectF viewRect = m_d->viewConverter->documentToView(docRect);
    m_d->canvasWidget->widget()->update( viewRect.toRect() );

/*
    kDebug(41010 ) << ">>>>>>>>>>>>>>>>>> canvas cache size: " << m_d->canvasCache.size() << endl;
    QLabel * l = new QLabel( 0 );
    l->setPixmap( QPixmap::fromImage( m_d->canvasCache ) );
    l->show();
*/
}


void KisCanvas2::updateCanvas()
{
    m_d->canvasWidget->widget()->update();
}


KoViewConverter* KisCanvas2::viewConverter()
{
    return m_d->viewConverter;
}

QWidget* KisCanvas2::canvasWidget()
{
    return m_d->canvasWidget->widget();
}


KoUnit KisCanvas2::unit()
{
    return KoUnit(KoUnit::Pixel);
}

KoToolProxy * KisCanvas2::toolProxy() {
    return m_d->canvasWidget->toolProxy();
}

void KisCanvas2::setCanvasSize(int w, int h)
{
    m_d->canvasWidget->widget()->setMinimumSize( w, h );
}

KisImageSP KisCanvas2::image()
{
    return m_d->view->image();

}

QImage KisCanvas2::canvasCache()
{
    return m_d->canvasCache;
}

KoColorProfile *  KisCanvas2::monitorProfile()
{
    if (m_d->monitorProfile == 0) {
        resetMonitorProfile();
    }
    return m_d->monitorProfile;
}


void KisCanvas2::resetMonitorProfile()
{
    m_d->monitorProfile = KoColorProfile::getScreenProfile();

    if (m_d->monitorProfile == 0) {
        KisConfig cfg;
        QString monitorProfileName = cfg.monitorProfile();
        m_d->monitorProfile = KoColorSpaceRegistry::instance()->profileByName(monitorProfileName);
    }
}

KisImageSP KisCanvas2::currentImage()
{
    return m_d->view->image();
}

void KisCanvas2::setImageSize( qint32 w, qint32 h )
{
    kDebug(41007) << "KisCanvas2::setImageSize " << w << ", " << h << endl;
    m_d->canvasCache = QImage( w, h, QImage::Format_ARGB32 );
}

void KisCanvas2::controllerSizeChanged( const QSize & size )
{
    if ( m_d->canvasWidget )
        m_d->canvasWidget->parentSizeChanged( size );
}

#if 0
// Old paint code from kis_view.cc
void KisView::zoomAroundPoint(double x, double y, double zf)
{
    // Disable updates while we change the scrollbar settings.
    m_canvas->setUpdatesEnabled(false);
    m_hScroll->setUpdatesEnabled(false);
    m_vScroll->setUpdatesEnabled(false);

    if (x < 0 || y < 0) {
        // Zoom about the centre of the current display
        KisImageSP img = currentImg();

        if (img) {
            if (m_hScroll->isVisible()) {
                KisPoint c = viewToWindow(KisPoint(m_canvas->width() / 2.0, m_canvas->height() / 2.0));
                x = c.x();
            }
            else {
                x = img->width() / 2.0;
            }

            if (m_vScroll->isVisible()) {
                KisPoint c = viewToWindow(KisPoint(m_canvas->width() / 2.0, m_canvas->height() / 2.0));
                y = c.y();
            }
            else {
                y = img->height() / 2.0;
            }
        }
        else {
            x = 0;
            y = 0;
        }
    }

    setZoom(zf);

    Q_ASSERT(m_zoomIn);
    Q_ASSERT(m_zoomOut);

    updateStatusBarZoomLabel ();

    m_zoomIn->setEnabled(zf < KISVIEW_MAX_ZOOM);
    m_zoomOut->setEnabled(zf > KISVIEW_MIN_ZOOM);
    resizeEvent(0);

    m_hRuler->setZoom(zf);
    m_vRuler->setZoom(zf);

    if (m_hScroll->isVisible()) {
        double vcx = m_canvas->width() / 2.0;
        Q_INT32 scrollX = qRound(x * zoom() - vcx);
        m_hScroll->setValue(scrollX);
    }

    if (m_vScroll->isVisible()) {
        double vcy = m_canvas->height() / 2.0;
        Q_INT32 scrollY = qRound(y * zoom() - vcy);
        m_vScroll->setValue(scrollY);
    }

    // Now update everything.
    m_canvas->setUpdatesEnabled(true);
    m_hScroll->setUpdatesEnabled(true);
    m_vScroll->setUpdatesEnabled(true);
    m_hScroll->update();
    m_vScroll->update();

    if (m_canvas->isOpenGLCanvas()) {
        paintOpenGLView(QRect(0, 0, m_canvas->width(), m_canvas->height()));
    } else {
        refreshKisCanvas();
    }

    emit viewTransformationsChanged();
}

void KisView::zoomTo(const KisRect& r)
{
    if (!r.isNull()) {

        double wZoom = fabs(m_canvas->width() / r.width());
        double hZoom = fabs(m_canvas->height() / r.height());

        double zf = kMin(wZoom, hZoom);

        if (zf < KISVIEW_MIN_ZOOM) {
            zf = KISVIEW_MIN_ZOOM;
        }
        else
            if (zf > KISVIEW_MAX_ZOOM) {
                zf = KISVIEW_MAX_ZOOM;
            }

        zoomAroundPoint(r.center().x(), r.center().y(), zf);
    }
}


void KisView::resizeEvent(QResizeEvent *)
{
    if (!m_paintViewEnabled) {
        startInitialZoomTimerIfReady();
    }

    KisImageSP img = currentImg();
    Q_INT32 scrollBarExtent = style().pixelMetric(QStyle::PM_ScrollBarExtent);
    Q_INT32 drawH;
    Q_INT32 drawW;
    Q_INT32 docW;
    Q_INT32 docH;

//    if (img) {
//        KisGuideMgr *mgr = img->guides();
//        mgr->resize(size());
//    }

    docW = static_cast<Q_INT32>(ceil(docWidth() * zoom()));
    docH = static_cast<Q_INT32>(ceil(docHeight() * zoom()));

    m_rulerThickness = m_RulerAction->isChecked() ? RULER_THICKNESS : 0;
    drawH = height() - m_rulerThickness;
    drawW = width() - m_rulerThickness;

    if (drawH < docH) {
        // Will need vert scrollbar
        drawW -= scrollBarExtent;
        if (drawW < docW)
            // Will need horiz scrollbar
            drawH -= scrollBarExtent;
    } else if (drawW < docW) {
        // Will need horiz scrollbar
        drawH -= scrollBarExtent;
        if (drawH < docH)
            // Will need vert scrollbar
            drawW -= scrollBarExtent;
    }

    m_vScroll->setEnabled(docH > drawH);
    m_hScroll->setEnabled(docW > drawW);

    if (docH <= drawH && docW <= drawW) {
        // we need no scrollbars
        m_vScroll->hide();
        m_hScroll->hide();
        m_vScroll->setValue(0);
        m_hScroll->setValue(0);
        m_vScrollBarExtent = 0;
        m_hScrollBarExtent = 0;
    } else if (docH <= drawH) {
        // we need a horizontal scrollbar only
        m_vScroll->hide();
        m_vScroll->setValue(0);
        m_hScroll->setRange(0, docW - drawW);
        m_hScroll->setGeometry(m_rulerThickness,
                     height() - scrollBarExtent,
                     width() - m_rulerThickness,
                     scrollBarExtent);
        m_hScroll->show();
        m_hScrollBarExtent = scrollBarExtent;
        m_hScrollBarExtent = scrollBarExtent;
    } else if(docW <= drawW) {
        // we need a vertical scrollbar only
        m_hScroll->hide();
        m_hScroll->setValue(0);
        m_vScroll->setRange(0, docH - drawH);
        m_vScroll->setGeometry(width() - scrollBarExtent, m_rulerThickness, scrollBarExtent, height()  - m_rulerThickness);
        m_vScroll->show();
        m_vScrollBarExtent = scrollBarExtent;
    } else {
        // we need both scrollbars
        m_vScroll->setRange(0, docH - drawH);
        m_vScroll->setGeometry(width() - scrollBarExtent,
                    m_rulerThickness,
                    scrollBarExtent,
                    height() -2* m_rulerThickness);
        m_hScroll->setRange(0, docW - drawW);
        m_hScroll->setGeometry(m_rulerThickness,
                     height() - scrollBarExtent,
                     width() - 2*m_rulerThickness,
                     scrollBarExtent);
        m_vScroll->show();
        m_hScroll->show();
        m_vScrollBarExtent = scrollBarExtent;
        m_hScrollBarExtent = scrollBarExtent;
    }

    Q_INT32 oldCanvasXOffset = m_canvasXOffset;
    Q_INT32 oldCanvasYOffset = m_canvasYOffset;

    if (docW < drawW) {
        m_canvasXOffset = (drawW - docW) / 2;
    } else {
        m_canvasXOffset = 0;
    }

    if (docH < drawH) {
        m_canvasYOffset = (drawH - docH) / 2;
    } else {
        m_canvasYOffset = 0;
    }

    //Check if rulers are visible
    if( m_RulerAction->isChecked() )
        m_canvas->setGeometry(m_rulerThickness, m_rulerThickness, drawW, drawH);
    else
        m_canvas->setGeometry(0, 0, drawW, drawH);
    m_canvas->show();

    if (!m_canvas->isOpenGLCanvas()) {

        if (m_canvasPixmap.size() != QSize(drawW, drawH)) {

            Q_INT32 oldCanvasWidth = m_canvasPixmap.width();
            Q_INT32 oldCanvasHeight = m_canvasPixmap.height();

            Q_INT32 newCanvasWidth = drawW;
            Q_INT32 newCanvasHeight = drawH;

            QRegion exposedRegion = QRect(0, 0, newCanvasWidth, newCanvasHeight);

            // Increase size first so that we can copy the old image area to the new one.
            m_canvasPixmap.resize(QMAX(oldCanvasWidth, newCanvasWidth), QMAX(oldCanvasHeight, newCanvasHeight));

            if (!m_canvasPixmap.isNull()) {

                if (oldCanvasXOffset != m_canvasXOffset || oldCanvasYOffset != m_canvasYOffset) {

                    Q_INT32 srcX;
                    Q_INT32 srcY;
                    Q_INT32 srcWidth;
                    Q_INT32 srcHeight;
                    Q_INT32 dstX;
                    Q_INT32 dstY;

                    if (oldCanvasXOffset <= m_canvasXOffset) {
                        // Move to the right
                        srcX = 0;
                        dstX = m_canvasXOffset - oldCanvasXOffset;
                        srcWidth = oldCanvasWidth;
                    } else {
                        // Move to the left
                        srcX = oldCanvasXOffset - m_canvasXOffset;
                        dstX = 0;
                        srcWidth = newCanvasWidth;
                    }

                    if (oldCanvasYOffset <= m_canvasYOffset) {
                        // Move down
                        srcY = 0;
                        dstY = m_canvasYOffset - oldCanvasYOffset;
                        srcHeight = oldCanvasHeight;
                    } else {
                        // Move up
                        srcY = oldCanvasYOffset - m_canvasYOffset;
                        dstY = 0;
                        srcHeight = newCanvasHeight;
                    }

                    bitBlt(&m_canvasPixmap, dstX, dstY, &m_canvasPixmap, srcX, srcY, srcWidth, srcHeight);
                    exposedRegion -= QRegion(QRect(dstX, dstY, srcWidth, srcHeight));
                } else {
                    exposedRegion -= QRegion(QRect(0, 0, oldCanvasWidth, oldCanvasHeight));
                }
            }

            m_canvasPixmap.resize(newCanvasWidth, newCanvasHeight);

            if (!m_canvasPixmap.isNull() && !exposedRegion.isEmpty()) {

                QMemArray<QRect> rects = exposedRegion.rects();

                for (unsigned int i = 0; i < rects.count(); i++) {
                    QRect r = rects[i];
                    updateQPaintDeviceCanvas(viewToWindow(r));
                }
            }
        }
    }

    int fontheight = QFontMetrics(KGlobalSettings::generalFont()).height() * 3;
    m_vScroll->setPageStep(drawH);
    m_vScroll->setLineStep(fontheight);
    m_hScroll->setPageStep(drawW);
    m_hScroll->setLineStep(fontheight);

    m_hRuler->setGeometry(m_rulerThickness + m_canvasXOffset, 0, QMIN(docW, drawW), m_rulerThickness);
    m_vRuler->setGeometry(0, m_rulerThickness + m_canvasYOffset, m_rulerThickness, QMIN(docH, drawH));

    if (m_vScroll->isVisible())
        m_vRuler->updateVisibleArea(0, m_vScroll->value());
    else
        m_vRuler->updateVisibleArea(0, 0);

    if (m_hScroll->isVisible())
        m_hRuler->updateVisibleArea(m_hScroll->value(), 0);
    else
        m_hRuler->updateVisibleArea(0, 0);

    if( m_RulerAction->isChecked() )
    {
        m_hRuler->show();
        m_vRuler->show();
    }
    else {
        m_hRuler->hide();
        m_vRuler->hide();
    }

    emit viewTransformationsChanged();
}


void KisView::updateQPaintDeviceCanvas(const QRect& imageRect)
{
    QRect vr = windowToView(imageRect);
    vr &= QRect(0, 0, m_canvas->width(), m_canvas->height());

    if (!vr.isEmpty()) {

        QPainter gc;

        if (gc.begin(&m_canvasPixmap)) {

            KisImageSP img = currentImg();

            if (img && m_paintViewEnabled) {

                QRect wr = viewToWindow(vr);

                if (wr.left() < 0 || wr.right() >= img->width() || wr.top() < 0 || wr.bottom() >= img->height()) {
                    // Erase areas outside document
                    QRegion rg(vr);
                    rg -= QRegion(windowToView(QRect(0, 0, img->width(), img->height())));

                    QMemArray<QRect> rects = rg.rects();

                    for (unsigned int i = 0; i < rects.count(); i++) {
                        QRect er = rects[i];
                        gc.fillRect(er, colorGroup().mid());
                    }
                    wr &= QRect(0, 0, img->width(), img->height());
                }

                if (!wr.isEmpty()) {

                    KisImage::PaintFlags paintFlags = (KisImage::PaintFlags)KisImage::PAINT_BACKGROUND;

                    if (m_actLayerVis) {
                        paintFlags = (KisImage::PaintFlags)(paintFlags|KisImage::PAINT_MASKINACTIVELAYERS);
                    }

                    if (m_selectionManager->displaySelection())
                    {
                        paintFlags = (KisImage::PaintFlags)(paintFlags|KisImage::PAINT_SELECTION);
                    }

                    if (zoom() > 1.0 - EPSILON) {

                        gc.setWorldXForm(true);
                        gc.translate(-horzValue(), -vertValue());
                        gc.scale(zoomFactor(), zoomFactor());

                        m_image->renderToPainter(wr.left(), wr.top(),
                            wr.right(), wr.bottom(), gc, monitorProfile(),
                            paintFlags, HDRExposure());
                    } else {

                        QRect canvasRect = windowToView(wr);
                        QRect scaledImageRect = canvasRect;
                        scaledImageRect.moveBy(horzValue(), vertValue());

                        QSize scaledImageSize(static_cast<Q_INT32>(ceil(docWidth() * zoom())),
                                            static_cast<Q_INT32>(ceil(docHeight() * zoom())));

                        QImage image = m_image->convertToQImage(scaledImageRect, scaledImageSize,
                                                                monitorProfile(), paintFlags, HDRExposure());

                        gc.drawImage(canvasRect.topLeft(), image, image.rect());

                        // Set up for the grid drawer.
                        gc.setWorldXForm(true);
                        gc.translate(-horzValue(), -vertValue());
                        gc.scale(zoomFactor(), zoomFactor());
                    }

                    m_gridManager->drawGrid( wr, &gc );
                    m_perspectiveGridManager->drawGrid( wr, &gc );
                }
//                    paintGuides();
            } else {
                gc.fillRect(vr, colorGroup().mid());
            }
        }
    }
}

void KisView::paintQPaintDeviceView(const QRegion& canvasRegion)
{
    Q_ASSERT(m_canvas->QPaintDeviceWidget() != 0);

    if (m_canvas->QPaintDeviceWidget() != 0 && !m_canvasPixmap.isNull()) {
        QMemArray<QRect> rects = canvasRegion.rects();

        for (unsigned int i = 0; i < rects.count(); i++) {
            QRect r = rects[i];

            bitBlt(m_canvas->QPaintDeviceWidget(), r.x(), r.y(), &m_canvasPixmap,
                   r.x(), r.y(), r.width(), r.height());
        }

        paintToolOverlay(canvasRegion);
    }
}

void KisView::updateOpenGLCanvas(const QRect& imageRect)
{
#ifdef HAVE_GL
    KisImageSP img = currentImg();

    if (img && m_paintViewEnabled) {
        Q_ASSERT(m_OpenGLImageContext != 0);

        if (m_OpenGLImageContext != 0) {
            m_OpenGLImageContext->update(imageRect);
        }
    }
#else
    Q_UNUSED(imageRect);
#endif
}

void KisView::paintOpenGLView(const QRect& canvasRect)
{
#ifdef HAVE_GL
    if (!m_canvas->isUpdatesEnabled()) {
        return;
    }

    m_canvas->OpenGLWidget()->makeCurrent();

    glDrawBuffer(GL_BACK);

    QColor widgetBackgroundColor = colorGroup().mid();

    glClearColor(widgetBackgroundColor.red() / 255.0, widgetBackgroundColor.green() / 255.0, widgetBackgroundColor.blue() / 255.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    KisImageSP img = currentImg();

    if (img && m_paintViewEnabled) {

        QRect vr = canvasRect;
        vr &= QRect(0, 0, m_canvas->width(), m_canvas->height());

        if (!vr.isNull()) {

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glViewport(0, 0, m_canvas->width(), m_canvas->height());
            glOrtho(0, m_canvas->width(), m_canvas->height(), 0, -1, 1);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glBindTexture(GL_TEXTURE_2D, m_OpenGLImageContext->backgroundTexture());

            glTranslatef(m_canvasXOffset, m_canvasYOffset, 0.0);

            glEnable(GL_TEXTURE_2D);
            glBegin(GL_QUADS);

            glTexCoord2f(0.0, 0.0);
            glVertex2f(0.0, 0.0);

            glTexCoord2f((img->width() * zoom()) / KisOpenGLImageContext::BACKGROUND_TEXTURE_WIDTH, 0.0);
            glVertex2f(img->width() * zoom(), 0.0);

            glTexCoord2f((img->width() * zoom()) / KisOpenGLImageContext::BACKGROUND_TEXTURE_WIDTH,
                         (img->height() * zoom()) / KisOpenGLImageContext::BACKGROUND_TEXTURE_HEIGHT);
            glVertex2f(img->width() * zoom(), img->height() * zoom());

            glTexCoord2f(0.0, (img->height() * zoom()) / KisOpenGLImageContext::BACKGROUND_TEXTURE_HEIGHT);
            glVertex2f(0.0, img->height() * zoom());

            glEnd();

            glTranslatef(-m_canvasXOffset, -m_canvasYOffset, 0.0);

            glTranslatef(-horzValue(), -vertValue(), 0.0);
            glScalef(zoomFactor(), zoomFactor(), 1.0);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            QRect wr = viewToWindow(QRect(0, 0, m_canvas->width(), m_canvas->height()));
            wr &= QRect(0, 0, img->width(), img->height());

            m_OpenGLImageContext->setHDRExposure(HDRExposure());

            m_canvas->OpenGLWidget()->makeCurrent();

            for (int x = (wr.left() / m_OpenGLImageContext->imageTextureTileWidth()) * m_OpenGLImageContext->imageTextureTileWidth();
                  x <= wr.right();
                  x += m_OpenGLImageContext->imageTextureTileWidth()) {
                for (int y = (wr.top() / m_OpenGLImageContext->imageTextureTileHeight()) * m_OpenGLImageContext->imageTextureTileHeight();
                      y <= wr.bottom();
                      y += m_OpenGLImageContext->imageTextureTileHeight()) {

                    glBindTexture(GL_TEXTURE_2D, m_OpenGLImageContext->imageTextureTile(x, y));

                    glBegin(GL_QUADS);

                    glTexCoord2f(0.0, 0.0);
                    glVertex2f(x, y);

                    glTexCoord2f(1.0, 0.0);
                    glVertex2f(x + m_OpenGLImageContext->imageTextureTileWidth(), y);

                    glTexCoord2f(1.0, 1.0);
                    glVertex2f(x + m_OpenGLImageContext->imageTextureTileWidth(), y + m_OpenGLImageContext->imageTextureTileHeight());

                    glTexCoord2f(0.0, 1.0);
                    glVertex2f(x, y + m_OpenGLImageContext->imageTextureTileHeight());

                    glEnd();
                }
            }

            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);

            m_gridManager->drawGrid(wr, 0, true);
            m_perspectiveGridManager->drawGrid( wr, 0, true );

            // Unbind the texture otherwise the ATI driver crashes when the canvas context is
            // made current after the textures are deleted following an image resize.
            glBindTexture(GL_TEXTURE_2D, 0);

            //paintGuides();
        }
    }

    m_canvas->OpenGLWidget()->swapBuffers();

    paintToolOverlay(QRegion(canvasRect));

#else
    Q_UNUSED(canvasRect);
#endif
}

void KisView::updateCanvas()
{
    if (m_image) {
        updateCanvas(m_image->bounds());
    }
}

void KisView::updateCanvas(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
    updateCanvas(QRect(x, y, w, h));
}

void KisView::updateCanvas(const QRect& imageRect)
{
    if (m_canvas->isOpenGLCanvas()) {
        updateOpenGLCanvas(imageRect);
        paintOpenGLView(windowToView(imageRect));
    } else {
        updateQPaintDeviceCanvas(imageRect);
        //m_canvas->update(windowToView(imageRect));
        m_canvas->repaint(windowToView(imageRect));
    }
}


void KisView::refreshKisCanvas()
{
    QRect imageRect = viewToWindow(QRect(0, 0, m_canvas->width(), m_canvas->height()));

    if (m_image) {
        imageRect |= m_image->bounds();
    }

    updateCanvas(imageRect);

    // Enable this if updateCanvas does an m_canvas->update()
    //m_canvas->repaint();
}

void KisView::paintToolOverlay(const QRegion& region)
{
    if (!region.isEmpty() && m_toolManager->currentTool() && !m_toolIsPainting) {
        KisCanvasPainter gc(m_canvas);

        gc.setClipRegion(region);
        gc.setClipping(true);

        // Prevent endless loop if the tool needs to have the canvas repainted
        m_toolIsPainting = true;
        m_toolManager->currentTool()->paint(gc, region.boundingRect());
        m_toolIsPainting = false;
    }
}

void KisView::canvasGotPaintEvent(QPaintEvent *event)
{
    if (m_canvas->isOpenGLCanvas()) {
        paintOpenGLView(event->rect());
    } else {
        paintQPaintDeviceView(event->region());
    }
}



void KisView::scrollH(int value)
{
    m_hRuler->updateVisibleArea(value, 0);

    int xShift = m_scrollX - value;
    m_scrollX = value;

    if (m_canvas->isUpdatesEnabled()) {
        if (xShift > 0) {

            if (m_canvas->isOpenGLCanvas()) {
                paintOpenGLView(QRect(0, 0, m_canvas->width(), m_canvas->height()));
            } else {
                QRect drawRect(0, 0, xShift, m_canvasPixmap.height());

                bitBlt(&m_canvasPixmap, xShift, 0, &m_canvasPixmap, 0, 0, m_canvasPixmap.width() - xShift, m_canvasPixmap.height());

                updateQPaintDeviceCanvas(viewToWindow(drawRect));
                m_canvas->repaint();
            }
        } else if (xShift < 0) {

            QRect drawRect(m_canvasPixmap.width() + xShift, 0, -xShift, m_canvasPixmap.height());

            if (m_canvas->isOpenGLCanvas()) {
                paintOpenGLView(QRect(0, 0, m_canvas->width(), m_canvas->height()));
            } else {
                bitBlt(&m_canvasPixmap, 0, 0, &m_canvasPixmap, -xShift, 0, m_canvasPixmap.width() + xShift, m_canvasPixmap.height());

                updateQPaintDeviceCanvas(viewToWindow(drawRect));
                m_canvas->repaint();
            }
        }
	if (m_oldTool) {
            KisCanvasPainter gc(m_canvas);
            m_oldTool->paint(gc);
        }
    }

    if (xShift != 0) {
        // XXX do sth with the childframe or so
    }
    emit viewTransformationsChanged();
}

void KisView::scrollV(int value)
{
    m_vRuler->updateVisibleArea(0, value);

    int yShift = m_scrollY - value;
    m_scrollY = value;

    if (m_canvas->isUpdatesEnabled()) {
        if (yShift > 0) {

            if (m_canvas->isOpenGLCanvas()) {
                paintOpenGLView(QRect(0, 0, m_canvas->width(), m_canvas->height()));
            } else {
                QRect drawRect(0, 0, m_canvasPixmap.width(), yShift);

                bitBlt(&m_canvasPixmap, 0, yShift, &m_canvasPixmap, 0, 0, m_canvasPixmap.width(), m_canvasPixmap.height() - yShift);

                updateQPaintDeviceCanvas(viewToWindow(drawRect));
                m_canvas->repaint();
            }
        } else if (yShift < 0) {

            if (m_canvas->isOpenGLCanvas()) {
                paintOpenGLView(QRect(0, 0, m_canvas->width(), m_canvas->height()));
            } else {
                QRect drawRect(0, m_canvasPixmap.height() + yShift, m_canvasPixmap.width(), -yShift);

                bitBlt(&m_canvasPixmap, 0, 0, &m_canvasPixmap, 0, -yShift, m_canvasPixmap.width(), m_canvasPixmap.height() + yShift);

                updateQPaintDeviceCanvas(viewToWindow(drawRect));
                m_canvas->repaint();
            }
        }
	if (m_oldTool) {
            KisCanvasPainter gc(m_canvas);
            m_oldTool->paint(gc);
        }
    }

    if (yShift != 0) {
        // XXX do sth with the childframe or so
    }
    emit viewTransformationsChanged();
}
#endif
#include "kis_canvas2.moc"
