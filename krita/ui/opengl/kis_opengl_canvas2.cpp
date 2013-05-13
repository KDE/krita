/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006-2013
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
#include <QBrush>
#include <QPainter>
#include <QPaintEvent>
#include <QPoint>
#include <QPainter>
#include <QMatrix>
#include <QDebug>

#include <QGLShaderProgram>
#include <QGLBuffer>
#include <QGLFramebufferObject>
#include <QGLContext>

#include <kstandarddirs.h>

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
#include "kis_canvas_resource_provider.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_debug.h"


#include "opengl/kis_opengl_canvas2_p.h"

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
        : displayShader(0)
        , vertexBuffer(0)
        , indexBuffer(0)
    {
    }

    ~Private() {
        delete displayShader;
        delete vertexBuffer;
        delete indexBuffer;
    }

    KisOpenGLImageTexturesSP openGLImageTextures;

    QGLShaderProgram *displayShader;
    QGLBuffer *vertexBuffer;
    QGLBuffer *indexBuffer;

};

KisOpenGLCanvas2::KisOpenGLCanvas2(KisCanvas2 *canvas, KisCoordinatesConverter *coordinatesConverter, QWidget *parent, KisOpenGLImageTexturesSP imageTextures)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent, KisOpenGL::sharedContextWidget())
    , KisCanvasWidgetBase(canvas, coordinatesConverter)
    , m_d(new Private())
{
    m_d->openGLImageTextures = imageTextures;

    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_NoSystemBackground);

    KisConfig cfg;
    imageTextures->generateCheckerTexture(createCheckersImage(cfg.checkSize()));
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
    initializeGLFunctions(KisOpenGL::sharedContextWidget()->context());

    if (!VSyncWorkaround::tryDisableVSync(this)) {
        qWarning();
        qWarning() << "WARNING: We didn't manage to switch off VSync on your graphics adapter.";
        qWarning() << "WARNING: It means either your hardware or driver doesn't support it,";
        qWarning() << "WARNING: or we just don't know about this hardware. Please report us a bug";
        qWarning() << "WARNING: with the output of \'glxinfo\' for your card.";
        qWarning();
        qWarning() << "WARNING: Trying to workaround it by disabling Double Buffering.";
        qWarning() << "WARNING: You may see some flickering when painting with some tools. It doesn't";
        qWarning() << "WARNING: affect the quality of the final image, though.";
        qWarning();

        QGLFormat format = this->format();
        format.setDoubleBuffer(false);
        setFormat(format);

        if (doubleBuffer()) {
            qCritical() << "CRITICAL: Failed to disable Double Buffering. Lines may look \"bended\" on your image.";
            qCritical() << "CRITICAL: Your graphics card or driver does not fully support Krita's OpenGL canvas.";
            qCritical() << "CRITICAL: For an optimal experience, please disable OpenGL";
            qCritical();
        }
    }

    initializeShaders();

}

void KisOpenGLCanvas2::resizeGL(int width, int height)
{
    glViewport(0, 0, (GLint)width, (GLint)height);
    coordinatesConverter()->setCanvasWidgetSize(QSize(width, height));
}

void KisOpenGLCanvas2::paintEvent(QPaintEvent *)
{
    // Draw the border (that is, clear the whole widget to the border color)
    QColor widgetBackgroundColor = borderColor();
    glClearColor(widgetBackgroundColor.redF(), widgetBackgroundColor.greenF(), widgetBackgroundColor.blueF(), 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Q_ASSERT(canvas()->image());

    if (canvas()->image()) {
        drawCheckers();
        drawImage();
        restoreGLState();

        QRect boundingRect = coordinatesConverter()->imageRectInWidgetPixels().toAlignedRect();

        QPainter gc(this);
        drawDecorations(gc, boundingRect);
        gc.end();
    } else {
        restoreGLState();
    }

}

//void KisOpenGLCanvas2::loadQTransform(QTransform transform)
//{
//    GLfloat matrix[16];
//    memset(matrix, 0, sizeof(GLfloat) * 16);

//    matrix[0] = transform.m11();
//    matrix[1] = transform.m12();

//    matrix[4] = transform.m21();
//    matrix[5] = transform.m22();

//    matrix[12] = transform.m31();
//    matrix[13] = transform.m32();

//    matrix[3] = transform.m13();
//    matrix[7] = transform.m23();

//    matrix[15] = transform.m33();

//    glLoadMatrixf(matrix);
//}

void KisOpenGLCanvas2::drawCheckers()
{
    KisCoordinatesConverter *converter = coordinatesConverter();

    QTransform textureTransform;
    QTransform modelTransform;
    QRectF textureRect;
    QRectF modelRect;
    converter->getOpenGLCheckersInfo(&textureTransform, &modelTransform, &textureRect, &modelRect);

//    qDebug() << "textureTransform" << textureTransform
//             << "modelTransform" << modelTransform
//             << "textureRect" << textureRect
//             << "modelRect" << modelRect;

    // Should be shader without color correction
    m_d->displayShader->bind();

    QMatrix4x4 model;//(modelTransform);
    m_d->displayShader->setUniformValue("modelMatrix", model);

    //Set view/projection matrices
    QMatrix4x4 view;//(textureTransform);
    m_d->displayShader->setUniformValue("viewMatrix", view);

    QMatrix4x4 proj;
    proj.ortho(0, 1, 0, 1, -1, 1);
    m_d->displayShader->setUniformValue("projectionMatrix", proj);

    //Setup the geometry for rendering
    m_d->vertexBuffer->bind();
    m_d->indexBuffer->bind();

    m_d->displayShader->setAttributeBuffer("vertex", GL_FLOAT, 0, 3);
    m_d->displayShader->enableAttributeArray("vertex");
    m_d->displayShader->setAttributeBuffer("uv0", GL_FLOAT, 12 * sizeof(float), 2);
    m_d->displayShader->enableAttributeArray("uv0");
    m_d->displayShader->setUniformValue("texture0", 0);

    qreal checkerSize = m_d->openGLImageTextures->checkerTextureSize();
    m_d->displayShader->setUniformValue("textureScale", QVector2D(width() / checkerSize, height() / checkerSize));

    // render checkers
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_d->openGLImageTextures->checkerTexture());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    m_d->displayShader->release();
}

void KisOpenGLCanvas2::drawImage()
{
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    KisCoordinatesConverter *converter = coordinatesConverter();

    QRectF widgetRect(0,0, width(), height());
    QRectF widgetRectInImagePixels = converter->documentToImage(converter->widgetToDocument(widgetRect));

    qreal scaleX, scaleY;
    converter->imageScale(&scaleX, &scaleY);

    QRect wr = widgetRectInImagePixels.toAlignedRect() &
            m_d->openGLImageTextures->storedImageBounds();


    // should be a shader with color correction
    m_d->displayShader->bind();

    QVector3D imageSize(m_d->openGLImageTextures->storedImageBounds().width(),
                        m_d->openGLImageTextures->storedImageBounds().height(),
                        0.f);

    QMatrix4x4 model;//(modelTransform);
    m_d->displayShader->setUniformValue("modelMatrix", model);
    model.scale(imageSize * scaleX);

    //Set view/projection matrices
    QMatrix4x4 view;//(textureTransform);
    m_d->displayShader->setUniformValue("viewMatrix", view);


    //Setup the geometry for rendering
    m_d->vertexBuffer->bind();
    m_d->indexBuffer->bind();

    m_d->displayShader->setAttributeBuffer("vertex", GL_FLOAT, 0, 3);
    m_d->displayShader->enableAttributeArray("vertex");
    m_d->displayShader->setAttributeBuffer("uv0", GL_FLOAT, 12 * sizeof(float), 2);
    m_d->displayShader->enableAttributeArray("uv0");
    m_d->displayShader->setUniformValue("texture0", 0);

//    m_d->openGLImageTextures->activateHDRExposureProgram();

    makeCurrent();

    int firstColumn = m_d->openGLImageTextures->xToCol(wr.left());
    int lastColumn = m_d->openGLImageTextures->xToCol(wr.right());
    int firstRow = m_d->openGLImageTextures->yToRow(wr.top());
    int lastRow = m_d->openGLImageTextures->yToRow(wr.bottom());

    QMatrix4x4 proj;
    proj.ortho(0, width(), 0, height(), -10, 10);
    m_d->displayShader->setUniformValue("projectionMatrix", proj);

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

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        }
    }

//    m_d->openGLImageTextures->deactivateHDRExposureProgram();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    // Unbind the texture otherwise the ATI driver crashes when the canvas context is
    // made current after the textures are deleted following an image resize.
    glBindTexture(GL_TEXTURE_2D, 0);
}

void KisOpenGLCanvas2::saveGLState()
{
//    Q_ASSERT(!m_d->GLStateSaved);

//    if (!m_d->GLStateSaved) {
//        m_d->GLStateSaved = true;

//        glPushAttrib(GL_ALL_ATTRIB_BITS);
//        glMatrixMode(GL_PROJECTION);
//        glPushMatrix();
//        glMatrixMode(GL_TEXTURE);
//        glPushMatrix();
//        glMatrixMode(GL_MODELVIEW);
//        glPushMatrix();

//        glGetIntegerv(GL_CURRENT_PROGRAM, &m_d->savedCurrentProgram);
//        glUseProgram(NO_PROGRAM);
//    }
}

void KisOpenGLCanvas2::restoreGLState()
{
//    Q_ASSERT(m_d->GLStateSaved);

//    if (m_d->GLStateSaved) {
//        m_d->GLStateSaved = false;

//        glMatrixMode(GL_PROJECTION);
//        glPopMatrix();
//        glMatrixMode(GL_TEXTURE);
//        glPopMatrix();
//        glMatrixMode(GL_MODELVIEW);
//        glPopMatrix();
//        glPopAttrib();

//        glUseProgram(m_d->savedCurrentProgram);
    //    }
}

void KisOpenGLCanvas2::initializeShaders()
{
    m_d->displayShader = new QGLShaderProgram();
    m_d->displayShader->addShaderFromSourceFile(QGLShader::Vertex, KGlobal::dirs()->findResource("data", "krita/shaders/gl2.vert"));
    m_d->displayShader->addShaderFromSourceFile(QGLShader::Fragment, KGlobal::dirs()->findResource("data", "krita/shaders/gl2.frag"));

    if (! m_d->displayShader->link()) {
        qDebug() << "OpenGL error" << glGetError();
        qFatal("Failed linking display shader");
    }

    m_d->vertexBuffer = new QGLBuffer(QGLBuffer::VertexBuffer);
    m_d->vertexBuffer->create();
    m_d->vertexBuffer->bind();

    QVector<float> vertices;
    /*
     *  0.0, 1.0  ---- 1.0, 1.0
     *     |              |
     *     |              |
     *  0.0, 0.0  ---- 1.0, 0.0
     */
    vertices << 0.0f << 0.0f << 0.0f;
    vertices << 0.0f << 1.0f << 0.0f;
    vertices << 1.0f << 0.0f << 0.0f;
    vertices << 1.0f << 1.0f << 0.0f;
    int vertSize = sizeof(float) * vertices.count();

    // coordinates to convert vertex points to a position in the texture. Follows order of corner
    // points in vertices
    QVector<float> uvs;
    uvs << 0.f << 0.f;
    uvs << 0.f << 1.f;
    uvs << 1.f << 0.f;
    uvs << 1.f << 1.f;
    int uvSize = sizeof(float) * uvs.count();

    m_d->vertexBuffer->allocate(vertSize + uvSize);
    m_d->vertexBuffer->write(0, reinterpret_cast<void*>(vertices.data()), vertSize);
    m_d->vertexBuffer->write(vertSize, reinterpret_cast<void*>(uvs.data()), uvSize);
    m_d->vertexBuffer->release();

    m_d->indexBuffer = new QGLBuffer(QGLBuffer::IndexBuffer);
    m_d->indexBuffer->create();
    m_d->indexBuffer->bind();

    QVector<uint> indices;
    // determines where opengl looks for vertex data. create two clockwise triangles from
    // the points.
    /*
     *  1->-3
     *  |\  |
     *  ^ \ v
     *  |  \|
     *  0...2
     */
    indices << 0 << 1 << 2 << 1 << 3 << 2;
    m_d->indexBuffer->allocate(reinterpret_cast<void*>(indices.data()), indices.size() * sizeof(uint));
    m_d->indexBuffer->release();
}

void KisOpenGLCanvas2::beginOpenGL(void)
{
//    saveGLState();
}

void KisOpenGLCanvas2::endOpenGL(void)
{
//    restoreGLState();
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
