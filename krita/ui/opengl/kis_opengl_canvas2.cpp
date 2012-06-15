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
#include <QTransform>

#include <kxmlguifactory.h>

#include "KoToolProxy.h"
#include "KoToolManager.h"
#include "KoColorSpace.h"
#include "KoShapeManager.h"

#include "kis_types.h"
#include <ko_favorite_resource_manager.h>
#include "canvas/kis_canvas2.h"
#include "kis_coordinates_converter.h"
#include "kis_image.h"
#include "opengl/kis_opengl.h"
#include "opengl/kis_opengl_image_textures.h"
#include "kis_view2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_debug.h"
#include "kis_selection_manager.h"
#include "kis_group_layer.h"

#define NEAR_VAL -1000.0
#define FAR_VAL 1000.0

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

namespace
{
    const GLuint NO_PROGRAM = 0;
}

struct KisOpenGLCanvas2::Private
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

KisOpenGLCanvas2::KisOpenGLCanvas2(KisCanvas2 * canvas, KisCoordinatesConverter *coordinatesConverter, QWidget * parent, KisOpenGLImageTexturesSP imageTextures)
        : QGLWidget(QGLFormat(QGL::SampleBuffers), parent, KisOpenGL::sharedContextWidget())
        , KisCanvasWidgetBase(canvas, coordinatesConverter)
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

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    slotConfigChanged();
}

KisOpenGLCanvas2::~KisOpenGLCanvas2()
{
    delete m_d;
}

void KisOpenGLCanvas2::initializeGL()
{
}

void KisOpenGLCanvas2::resizeGL(int width, int height)
{
    glViewport(0, 0, (GLint)width, (GLint)height);
    coordinatesConverter()->setCanvasWidgetSize(QSize(width, height));
    emit needAdjustOrigin();
}

void KisOpenGLCanvas2::paintEvent(QPaintEvent *)
{
    QPainter gc(this);

    saveGLState();

    drawBorder();

    Q_ASSERT(canvas()->image());

    if (canvas()->image()) {
        drawBackground();
        drawImage();
        restoreGLState();


        QRect boundingRect = coordinatesConverter()->imageRectInWidgetPixels().toAlignedRect();
        drawDecorations(gc, boundingRect);
    } else {
        restoreGLState();
    }

    gc.end();
}

void KisOpenGLCanvas2::loadQTransform(QTransform transform)
{
    GLfloat matrix[16];
    memset(matrix, 0, sizeof(GLfloat) * 16);

    matrix[0] = transform.m11();
    matrix[1] = transform.m12();

    matrix[4] = transform.m21();
    matrix[5] = transform.m22();

    matrix[12] = transform.m31();
    matrix[13] = transform.m32();

    matrix[3] = transform.m13();
    matrix[7] = transform.m23();

    matrix[15] = transform.m33();

    glLoadMatrixf(matrix);
}

void KisOpenGLCanvas2::drawBorder()
{
    QColor widgetBackgroundColor = borderColor();

    glClearColor(widgetBackgroundColor.redF(),widgetBackgroundColor.greenF(),widgetBackgroundColor.blueF(),1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void KisOpenGLCanvas2::drawBackground()
{
    KisCoordinatesConverter *converter = coordinatesConverter();

    QTransform textureTransform;
    QTransform modelTransform;
    QRectF textureRect;
    QRectF modelRect;
    converter->getOpenGLCheckersInfo(&textureTransform, &modelTransform, &textureRect, &modelRect);

    KisConfig cfg;
    GLfloat checkSizeScale = KisOpenGLImageTextures::BACKGROUND_TEXTURE_CHECK_SIZE / static_cast<GLfloat>(cfg.checkSize());

    textureTransform *= QTransform::fromScale(checkSizeScale / KisOpenGLImageTextures::BACKGROUND_TEXTURE_SIZE,
                                               checkSizeScale / KisOpenGLImageTextures::BACKGROUND_TEXTURE_SIZE);


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, width(), height());
    glOrtho(0, width(), height(), 0, NEAR_VAL, FAR_VAL);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    loadQTransform(textureTransform);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    loadQTransform(modelTransform);

    glBindTexture(GL_TEXTURE_2D, m_d->openGLImageTextures->backgroundTexture());
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_FLAT);

    glBegin(GL_QUADS);
    glColor3f(1.0, 1.0, 1.0);

    glTexCoord2f(textureRect.left(), textureRect.top());
    glVertex2f(modelRect.left(), modelRect.top());

    glTexCoord2f(textureRect.right(), textureRect.top());
    glVertex2f(modelRect.right(), modelRect.top());

    glTexCoord2f(textureRect.right(), textureRect.bottom());
    glVertex2f(modelRect.right(), modelRect.bottom());

    glTexCoord2f(textureRect.left(), textureRect.bottom());
    glVertex2f(modelRect.left(), modelRect.bottom());

    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void KisOpenGLCanvas2::drawImage()
{
    setupImageToWidgetTransformation();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    KisImageWSP image = canvas()->image();
    KisCoordinatesConverter *converter = coordinatesConverter();

    QRectF widgetRect(0,0, width(), height());
    QRectF widgetRectInImagePixels = converter->
        documentToImage(converter->widgetToDocument(widgetRect));

    qreal scaleX, scaleY;
    converter->imageScale(&scaleX, &scaleY);

    QRect wr = widgetRectInImagePixels.toAlignedRect() &
        m_d->openGLImageTextures->storedImageBounds();

    if (image->colorSpace()->hasHighDynamicRange() && m_d->openGLImageTextures->usingHDRExposureProgram()) {
        m_d->openGLImageTextures->activateHDRExposureProgram();
    }

    makeCurrent();

    int firstColumn = m_d->openGLImageTextures->xToCol(wr.left());
    int lastColumn = m_d->openGLImageTextures->xToCol(wr.right());
    int firstRow = m_d->openGLImageTextures->yToRow(wr.top());
    int lastRow = m_d->openGLImageTextures->yToRow(wr.bottom());

    for (int col = firstColumn; col <= lastColumn; col++) {
        for (int row = firstRow; row <= lastRow; row++) {

            KisTextureTile *tile =
                m_d->openGLImageTextures->getTextureTileCR(col, row);

            glBindTexture(GL_TEXTURE_2D, tile->textureId());

            if(scaleX > 2.0) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }

            tile->drawPoints();
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
}

void KisOpenGLCanvas2::endOpenGL(void)
{
    restoreGLState();
}

void KisOpenGLCanvas2::setupImageToWidgetTransformation()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, width(), height());
    glOrtho(0, width(), height(), 0, NEAR_VAL, FAR_VAL);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);

    QTransform transform = coordinatesConverter()->imageToWidgetTransform();
    loadQTransform(transform);
}

void KisOpenGLCanvas2::setupFlakeToWidgetTransformation()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, width(), height());
    glOrtho(0, width(), height(), 0, NEAR_VAL, FAR_VAL);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);

    QTransform transform = coordinatesConverter()->flakeToWidgetTransform();
    loadQTransform(transform);
}

bool KisOpenGLCanvas2::event(QEvent *e)
{
    if(toolProxy()) {
        toolProxy()->processEvent(e);
    }
    return QWidget::event(e);
}

void KisOpenGLCanvas2::slotConfigChanged()
{
    notifyConfigChanged();
}

QVariant KisOpenGLCanvas2::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return processInputMethodQuery(query);
}

void KisOpenGLCanvas2::inputMethodEvent(QInputMethodEvent *event)
{
    processInputMethodEvent(event);
}

bool KisOpenGLCanvas2::callFocusNextPrevChild(bool next)
{
    return focusNextPrevChild(next);
}

#include "kis_opengl_canvas2.moc"
#endif // HAVE_OPENGL
