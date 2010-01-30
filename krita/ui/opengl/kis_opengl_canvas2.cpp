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
    Private()
            : savedCurrentProgram(NO_PROGRAM)
            , GLStateSaved(false)
    {
    }

    KisOpenGLImageTexturesSP openGLImageTextures;
    GLint savedCurrentProgram;
    bool GLStateSaved;
};

KisOpenGLCanvas2::KisOpenGLCanvas2(KisCanvas2 * canvas, QWidget * parent, KisOpenGLImageTexturesSP imageTextures)
        : QGLWidget(QGLFormat(QGL::SampleBuffers), parent, KisOpenGL::sharedContextWidget())
        , KisCanvasWidgetBase(canvas)
        , m_d(new Private())
{
    m_d->openGLImageTextures = imageTextures;

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

    if (canvas()->image()) {
        drawBackground();
        drawImage();

        restoreGLState();

        // XXX: make settable
        bool drawTools = true;

        drawDecorations(gc, drawTools,
                        documentOffset(),
                        QRect(QPoint(0, 0), QSize(width(), height())),
                        canvas());
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
        glTranslatef(static_cast<GLfloat>(documentOffset().x()) / KisOpenGLImageTextures::BACKGROUND_TEXTURE_SIZE,
                     static_cast<GLfloat>(documentOffset().y()) / KisOpenGLImageTextures::BACKGROUND_TEXTURE_SIZE,
                     0.0);
    }

    glScalef(scaleX, scaleY, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(documentOrigin().x(), documentOrigin().y(), 0.0);
    glScalef(scaleX, scaleY, 1.0);

    glBindTexture(GL_TEXTURE_2D, m_d->openGLImageTextures->backgroundTexture());
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_FLAT);

    glBegin(GL_QUADS);

    glColor3f(1.0, 1.0, 1.0);
    glTexCoord2f(0.0, 0.0);
    glVertex2f(0.0, 0.0);

    KisImageWSP image = canvas()->image();

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

    QRectF documentRect = viewConverter()->viewToDocument(QRectF(documentOffset().x(), 
                                                                    documentOffset().y(), 
                                                                    width(), 
                                                                    height()));
    KisImageWSP image = canvas()->image();

    QRect wr = image->documentToIntPixel(documentRect);
    wr &= QRect(0, 0, image->width(), image->height());

    if (image->colorSpace()->hasHighDynamicRange()) {
        if (m_d->openGLImageTextures->usingHDRExposureProgram()) {
            m_d->openGLImageTextures->activateHDRExposureProgram();
        }
        m_d->openGLImageTextures->setHDRExposure(canvas()->view()->resourceProvider()->HDRExposure());
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
    glTranslatef(-documentOffset().x(), -documentOffset().y(), 0.0);
    glTranslatef(documentOrigin().x(),  documentOrigin().y(), 0.0);
}

void KisOpenGLCanvas2::applyZoomScalingToModelView(void)
{
    KisImageWSP image = canvas()->image();
    
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
    viewConverter()->zoom(&sx, &sy);

    return sx / canvas()->image()->xRes();
}

qreal KisOpenGLCanvas2::zoomScaleY() const
{
    qreal sx, sy;
    viewConverter()->zoom(&sx, &sy);

    return sy / canvas()->image()->yRes();
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
    KisCanvasWidgetBase::mouseMoveEvent(e);
}

void KisOpenGLCanvas2::contextMenuEvent(QContextMenuEvent *e)
{
    KisCanvasWidgetBase::contextMenuEvent(e);
}

void KisOpenGLCanvas2::mousePressEvent(QMouseEvent *e)
{
    KisCanvasWidgetBase::mousePressEvent(e);
}

void KisOpenGLCanvas2::mouseReleaseEvent(QMouseEvent *e)
{
    KisCanvasWidgetBase::mouseReleaseEvent(e);
}

void KisOpenGLCanvas2::mouseDoubleClickEvent(QMouseEvent *e)
{
    KisCanvasWidgetBase::mouseDoubleClickEvent(e);
}

void KisOpenGLCanvas2::keyPressEvent(QKeyEvent *e)
{
    KisCanvasWidgetBase::keyPressEvent(e);
}

void KisOpenGLCanvas2::keyReleaseEvent(QKeyEvent *e)
{
    KisCanvasWidgetBase::keyReleaseEvent(e);
}

QVariant KisOpenGLCanvas2::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return KisCanvasWidgetBase::inputMethodQuery(query);
}

void KisOpenGLCanvas2::inputMethodEvent(QInputMethodEvent *event)
{
    KisCanvasWidgetBase::inputMethodEvent(event);
}

void KisOpenGLCanvas2::tabletEvent(QTabletEvent *e)
{
    KisCanvasWidgetBase::tabletEvent(e);
}

void KisOpenGLCanvas2::wheelEvent(QWheelEvent *e)
{
    KisCanvasWidgetBase::wheelEvent(e);
}

void KisOpenGLCanvas2::documentOffsetMoved(const QPoint & pt)
{
    KisCanvasWidgetBase::documentOffsetMoved(pt);
    draw();
}

void KisOpenGLCanvas2::emitDocumentOriginChangedSignal()
{
    emit documentOriginChanged(documentOrigin());
}

bool KisOpenGLCanvas2::callFocusNextPrevChild(bool next)
{
    return focusNextPrevChild(next);
}

#include "kis_opengl_canvas2.moc"
#endif // HAVE_OPENGL
