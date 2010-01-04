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

#include "opengl/kis_opengl_canvas2.h"


#ifdef HAVE_OPENGL

#include <QMenu>
#include <QWidget>
#include <QGLWidget>
#include <QGLContext>
#include <QImage>
#include <QBrush>
#include <QPainter>
#include <QPaintEvent>
#include <QPoint>
#include <QPainter>
#include <QTimer>

#include <kxmlguifactory.h>

#include "KoToolProxy.h"
#include "KoToolManager.h"
#include "KoColorSpace.h"
#include "KoShapeManager.h"

#include "kis_types.h"
#include <ko_favorite_resource_manager.h>
#include "canvas/kis_canvas2.h"
#include "kis_image.h"
#include "opengl/kis_opengl.h"
#include "opengl/kis_opengl_image_textures.h"
#include "kis_view2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_config.h"
#include "kis_debug.h"
#include "kis_selection_manager.h"

#define NEAR_VAL -10.0
#define FAR_VAL 10.0

namespace
{
    const GLuint NO_PROGRAM = 0;
}

class KisOpenGLCanvas2::Private
{
public:
    Private(const KoViewConverter *vc)
            : viewConverter(vc)
            , canvas(0)
            , toolProxy(0)
            , savedCurrentProgram(NO_PROGRAM)
            , GLStateSaved(false)
    {
    }

    const KoViewConverter * viewConverter;
    KisCanvas2 * canvas;
    KoToolProxy * toolProxy;
    KisOpenGLImageTexturesSP openGLImageTextures;
    /// the origin of the image rect
    QPoint origin;
    QPoint documentOffset;
    QTimer blockMouseEvent;
    GLint savedCurrentProgram;
    bool GLStateSaved;
};

KisOpenGLCanvas2::KisOpenGLCanvas2(KisCanvas2 * canvas, QWidget * parent, KisOpenGLImageTexturesSP imageTextures)
        : QGLWidget(QGLFormat(QGL::SampleBuffers), parent, KisOpenGL::sharedContextWidget())
        , m_d(new Private(canvas->viewConverter()))
{
    m_d->canvas = canvas;
    m_d->toolProxy = canvas->toolProxy();
    m_d->openGLImageTextures = imageTextures;
    m_d->blockMouseEvent.setSingleShot(true);

    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_NoSystemBackground);
    imageTextures->generateBackgroundTexture(checkImage(KisOpenGLImageTextures::BACKGROUND_TEXTURE_CHECK_SIZE));
    setAttribute(Qt::WA_InputMethodEnabled, true);

    if (isSharing()) {
        dbgUI << "Created QGLWidget with sharing";
    } else {
        dbgUI << "Created QGLWidget with no sharing";
    }
}

KisOpenGLCanvas2::~KisOpenGLCanvas2()
{
    delete m_d;
}

void KisOpenGLCanvas2::initializeGL()
{
}

void KisOpenGLCanvas2::resizeGL(int w, int h)
{
    glViewport(0, 0, (GLint)w, (GLint)h);
    adjustOrigin();
    draw();
}

void KisOpenGLCanvas2::paintEvent(QPaintEvent *)
{
    draw();
}

void KisOpenGLCanvas2::draw()
{
    QPainter gc(this);

    saveGLState();

    drawBorder();

    if (m_d->canvas->image()) {
        drawBackground();
        drawImage();

        restoreGLState();

        // XXX: make settable
        bool drawTools = true;

        drawDecorations(gc, drawTools,
                        m_d->documentOffset,
                        QRect(QPoint(0, 0), QSize(width(), height())),
                        m_d->canvas);
    } else {
        restoreGLState();
    }

    gc.end();
}

void KisOpenGLCanvas2::drawBorder()
{
    QColor widgetBackgroundColor = borderColor();

    glClearColor(widgetBackgroundColor.red() / 255.0, widgetBackgroundColor.green() / 255.0, 
                 widgetBackgroundColor.blue() / 255.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void KisOpenGLCanvas2::drawBackground()
{
    const qreal scaleX = zoomScaleX();
    const qreal scaleY = zoomScaleY();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, width(), height());
    glOrtho(0, width(), height(), 0, NEAR_VAL, FAR_VAL);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    KisConfig cfg;
    GLfloat checkSizeScale = KisOpenGLImageTextures::BACKGROUND_TEXTURE_CHECK_SIZE / static_cast<GLfloat>(cfg.checkSize());

    glScalef(checkSizeScale, checkSizeScale, 1.0);

    if (cfg.scrollCheckers()) {
        glTranslatef(static_cast<GLfloat>(m_d->documentOffset.x()) / KisOpenGLImageTextures::BACKGROUND_TEXTURE_SIZE,
                     static_cast<GLfloat>(m_d->documentOffset.y()) / KisOpenGLImageTextures::BACKGROUND_TEXTURE_SIZE,
                     0.0);
    }

    glScalef(scaleX, scaleY, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(m_d->origin.x(), m_d->origin.y(), 0.0);
    glScalef(scaleX, scaleY, 1.0);

    glBindTexture(GL_TEXTURE_2D, m_d->openGLImageTextures->backgroundTexture());
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_FLAT);

    glBegin(GL_QUADS);

    glColor3f(1.0, 1.0, 1.0);
    glTexCoord2f(0.0, 0.0);
    glVertex2f(0.0, 0.0);

    KisImageWSP image = m_d->canvas->image();

    glTexCoord2f(image->width() / KisOpenGLImageTextures::BACKGROUND_TEXTURE_SIZE, 0.0);
    glVertex2f(image->width(), 0.0);

    glTexCoord2f(image->width() / KisOpenGLImageTextures::BACKGROUND_TEXTURE_SIZE,
                 image->height() / KisOpenGLImageTextures::BACKGROUND_TEXTURE_SIZE);
    glVertex2f(image->width(), image->height());

    glTexCoord2f(0.0, image->height() / KisOpenGLImageTextures::BACKGROUND_TEXTURE_SIZE);
    glVertex2f(0.0, image->height());

    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void KisOpenGLCanvas2::drawImage()
{
    setPixelToViewTransformation();

    const qreal scaleX = zoomScaleX();

    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    QRectF documentRect = m_d->viewConverter->viewToDocument(QRectF(m_d->documentOffset.x(), 
                                                                    m_d->documentOffset.y(), 
                                                                    width(), 
                                                                    height()));
    KisImageWSP image = m_d->canvas->image();

    QRect wr = image->documentToIntPixel(documentRect);
    wr &= QRect(0, 0, image->width(), image->height());

    if (image->colorSpace()->hasHighDynamicRange()) {
        if (m_d->openGLImageTextures->usingHDRExposureProgram()) {
            m_d->openGLImageTextures->activateHDRExposureProgram();
        }
        m_d->openGLImageTextures->setHDRExposure(m_d->canvas->view()->resourceProvider()->HDRExposure());
    }

    makeCurrent();

    for (int x = (wr.left() / m_d->openGLImageTextures->imageTextureTileWidth()) * m_d->openGLImageTextures->imageTextureTileWidth();
        x <= wr.right();
        x += m_d->openGLImageTextures->imageTextureTileWidth()) {
        for (int y = (wr.top() / m_d->openGLImageTextures->imageTextureTileHeight()) * m_d->openGLImageTextures->imageTextureTileHeight();
            y <= wr.bottom();
            y += m_d->openGLImageTextures->imageTextureTileHeight()) {

            glBindTexture(GL_TEXTURE_2D, m_d->openGLImageTextures->imageTextureTile(x, y));
            if (scaleX > 2.0) {
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            } else {
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }

            glBegin(GL_QUADS);

            glTexCoord2f(0.0, 0.0);
            glVertex2f(x, y);

            glTexCoord2f(1.0, 0.0);
            glVertex2f(x + m_d->openGLImageTextures->imageTextureTileWidth(), y);

            glTexCoord2f(1.0, 1.0);
            glVertex2f(x + m_d->openGLImageTextures->imageTextureTileWidth(), y + m_d->openGLImageTextures->imageTextureTileHeight());

            glTexCoord2f(0.0, 1.0);
            glVertex2f(x, y + m_d->openGLImageTextures->imageTextureTileHeight());

            glEnd();
        }
    }

    if (image->colorSpace()->hasHighDynamicRange()) {
        if (m_d->openGLImageTextures->usingHDRExposureProgram()) {
            m_d->openGLImageTextures->deactivateHDRExposureProgram();
        }
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    // Unbind the texture otherwise the ATI driver crashes when the canvas context is
    // made current after the textures are deleted following an image resize.
    glBindTexture(GL_TEXTURE_2D, 0);
}

void KisOpenGLCanvas2::saveGLState()
{
    Q_ASSERT(!m_d->GLStateSaved);

    if (!m_d->GLStateSaved) {
        m_d->GLStateSaved = true;

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glMatrixMode(GL_TEXTURE);
        glPushMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();

#ifdef HAVE_GLEW
        if (KisOpenGL::hasShadingLanguage()) {
            glGetIntegerv(GL_CURRENT_PROGRAM, &m_d->savedCurrentProgram);
            glUseProgram(NO_PROGRAM);
        }
#endif
    }
}

void KisOpenGLCanvas2::restoreGLState()
{
    Q_ASSERT(m_d->GLStateSaved);

    if (m_d->GLStateSaved) {
        m_d->GLStateSaved = false;

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_TEXTURE);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glPopAttrib();

#ifdef HAVE_GLEW
        if (KisOpenGL::hasShadingLanguage()) {
            glUseProgram(m_d->savedCurrentProgram);
        }
#endif
    }
}

void KisOpenGLCanvas2::beginOpenGL(void)
{
    saveGLState();
    setupMatrices();
}

void KisOpenGLCanvas2::endOpenGL(void)
{
    restoreGLState();
}

void KisOpenGLCanvas2::setupMatrices(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, width(), height());
    glOrtho(0, width(), height(), 0, NEAR_VAL, FAR_VAL);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-m_d->documentOffset.x(), -m_d->documentOffset.y(), 0.0);
    glTranslatef(m_d->origin.x(),  m_d->origin.y(), 0.0);
}

void KisOpenGLCanvas2::applyZoomScalingToModelView(void)
{
    KisImageWSP image = m_d->canvas->image();
    
    if (!image) return;
    
    const qreal scaleX = zoomScaleX();
    const qreal scaleY = zoomScaleY();
    
    glMatrixMode(GL_MODELVIEW);
    glScalef(scaleX, scaleY, 1.0);
}

void KisOpenGLCanvas2::setPixelToViewTransformation(void)
{
    setupMatrices();
    applyZoomScalingToModelView();
}

qreal KisOpenGLCanvas2::zoomScaleX() const
{
    qreal sx, sy;
    m_d->viewConverter->zoom(&sx, &sy);

    return sx / m_d->canvas->image()->xRes();
}

qreal KisOpenGLCanvas2::zoomScaleY() const
{
    qreal sx, sy;
    m_d->viewConverter->zoom(&sx, &sy);

    return sy / m_d->canvas->image()->yRes();
}

void KisOpenGLCanvas2::enterEvent(QEvent* e)
{
    QWidget::enterEvent(e);
}

void KisOpenGLCanvas2::leaveEvent(QEvent* e)
{
    draw();
    QWidget::leaveEvent(e);
}

void KisOpenGLCanvas2::mouseMoveEvent(QMouseEvent *e)
{
    if (m_d->blockMouseEvent.isActive()) return;
    m_d->toolProxy->mouseMoveEvent(e, m_d->viewConverter->viewToDocument(widgetToView(e->pos() + m_d->documentOffset)));
}

void KisOpenGLCanvas2::contextMenuEvent(QContextMenuEvent *e)
{
    m_d->canvas->view()->unplugActionList("flake_tool_actions");
    m_d->canvas->view()->plugActionList("flake_tool_actions",
                                        m_d->toolProxy->popupActionList());
    QMenu *menu = dynamic_cast<QMenu*>(m_d->canvas->view()->factory()->container("image_popup", m_d->canvas->view()));
    if (menu)
        menu->exec(e->globalPos());
}

void KisOpenGLCanvas2::mousePressEvent(QMouseEvent *e)
{
    if (m_d->blockMouseEvent.isActive()) return;
    else if (m_d->canvas->view()->favoriteResourceManager()->isPopupPaletteVisible())
    {
        m_d->canvas->view()->favoriteResourceManager()->slotShowPopupPalette();
        return;
    }
    m_d->toolProxy->mousePressEvent(e, m_d->viewConverter->viewToDocument(widgetToView(e->pos() + m_d->documentOffset)));
}

void KisOpenGLCanvas2::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_d->blockMouseEvent.isActive()) return;
    m_d->toolProxy->mouseReleaseEvent(e, m_d->viewConverter->viewToDocument(widgetToView(e->pos() + m_d->documentOffset)));
}

void KisOpenGLCanvas2::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (m_d->blockMouseEvent.isActive()) return;
    m_d->toolProxy->mouseDoubleClickEvent(e, m_d->viewConverter->viewToDocument(widgetToView(e->pos() + m_d->documentOffset)));

}

void KisOpenGLCanvas2::keyPressEvent(QKeyEvent *e)
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

void KisOpenGLCanvas2::keyReleaseEvent(QKeyEvent *e)
{
    m_d->toolProxy->keyReleaseEvent(e);
}

QVariant KisOpenGLCanvas2::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return m_d->toolProxy->inputMethodQuery(query, *m_d->viewConverter);
}

void KisOpenGLCanvas2::inputMethodEvent(QInputMethodEvent *event)
{
    m_d->toolProxy->inputMethodEvent(event);
}

void KisOpenGLCanvas2::tabletEvent(QTabletEvent *e)
{
    setFocus(Qt::OtherFocusReason);
    m_d->blockMouseEvent.start(100);

    m_d->toolProxy->tabletEvent(e, m_d->viewConverter->viewToDocument(e->hiResGlobalPos() - mapToGlobal(QPoint(0, 0)) + m_d->documentOffset - m_d->origin));
    /*
    setFocus(Qt::OtherFocusReason);
    m_d->blockMouseEvent.start(100);
    qreal subpixelX = e->hiResGlobalX();
    subpixelX = subpixelX - ((int) subpixelX); // leave only part behind the dot
    qreal subpixelY = e->hiResGlobalY();
    subpixelY = subpixelY - ((int) subpixelY); // leave only part behind the dot
    QPointF pos(e->x() + subpixelX + m_d->documentOffset.x() - m_d->origin.x(), e->y() + subpixelY + m_d->documentOffset.y() - m_d->origin.y() );
    m_d->toolProxy->tabletEvent(e, m_d->viewConverter->viewToDocument(pos));
    */
}

void KisOpenGLCanvas2::wheelEvent(QWheelEvent *e)
{
    m_d->toolProxy->wheelEvent(e, m_d->viewConverter->viewToDocument(widgetToView(e->pos() + m_d->documentOffset)));
}

KoToolProxy * KisOpenGLCanvas2::toolProxy()
{
    return m_d->toolProxy;
}

void KisOpenGLCanvas2::documentOffsetMoved(const QPoint & pt)
{
    m_d->documentOffset = pt;
    draw();
}

void KisOpenGLCanvas2::adjustOrigin()
{
    KisImageWSP image = m_d->canvas->image();
    if (image == 0) return;

    QSize documentSize(int(ceil(m_d->viewConverter->documentToViewX(image->width()  / image->xRes()))),
                       int(ceil(m_d->viewConverter->documentToViewY(image->height() / image->yRes()))));
    QRect documentRect = QRect(QPoint(0, 0), documentSize);

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

QPoint KisOpenGLCanvas2::documentOrigin()
{
    return m_d->origin;
}

QPoint KisOpenGLCanvas2::widgetToView(const QPoint& p) const
{
    return p - m_d->origin;
}


QRect KisOpenGLCanvas2::widgetToView(const QRect& r) const
{
    return r.translated(- m_d->origin);
}


QPoint KisOpenGLCanvas2::viewToWidget(const QPoint& p) const
{
    return p + m_d->origin;
}


QRect KisOpenGLCanvas2::viewToWidget(const QRect& r) const
{
    return r.translated(m_d->origin);
}


#include "kis_opengl_canvas2.moc"
#endif // HAVE_OPENGL
