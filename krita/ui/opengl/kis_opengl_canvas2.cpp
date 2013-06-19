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
#include "opengl/kis_opengl.h"

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
#include <QThread>

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
#include "kis_coordinates_converter.h"
#include "canvas/kis_display_filter.h"

#define NEAR_VAL -1000.0
#define FAR_VAL 1000.0

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

namespace
{
const GLuint NO_PROGRAM = 0;
}

struct KisOpenGLCanvas2::Private
{
public:
    Private()
        : displayShader(0)
        , checkerShader(0)
        , displayFilter(0)
        , checkerVertexBuffer(0)
        , tileVertexBuffer(0)
    {
    }

    ~Private() {
        delete displayShader;
        delete checkerShader;
        delete checkerVertexBuffer;
        delete tileVertexBuffer;
    }

    KisOpenGLImageTexturesSP openGLImageTextures;

    QGLShaderProgram *displayShader;
    QGLShaderProgram *checkerShader;

    KisDisplayFilter *displayFilter;

    QGLBuffer *checkerVertexBuffer;
    QGLBuffer *tileVertexBuffer;
};

KisOpenGLCanvas2::KisOpenGLCanvas2(KisCanvas2 *canvas, KisCoordinatesConverter *coordinatesConverter, QWidget *parent, KisOpenGLImageTexturesSP imageTextures)
    : QGLWidget(KisOpenGL::sharedContextWidget()->format(), parent, KisOpenGL::sharedContextWidget())
    , KisCanvasWidgetBase(canvas, coordinatesConverter)
    , d(new Private())
{
    d->openGLImageTextures = imageTextures;

    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);

    setAttribute(Qt::WA_InputMethodEnabled, true);

    if (isSharing()) {
        dbgUI << "Created QGLWidget with sharing";
    } else {
        dbgUI << "Created QGLWidget with no sharing";
    }

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    slotConfigChanged();

    KisConfig cfg;
    d->openGLImageTextures->generateCheckerTexture(createCheckersImage(cfg.checkSize()));

}

KisOpenGLCanvas2::~KisOpenGLCanvas2()
{
    delete d;
}

void KisOpenGLCanvas2::setDisplayFilter(KisDisplayFilter *displayFilter)
{
    d->displayFilter = displayFilter;
    initializeDisplayShader();
}

void KisOpenGLCanvas2::initializeGL()
{
    glEnable(GL_MULTISAMPLE);
#ifndef Q_WS_WIN
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
#endif
        if (doubleBuffer()) {
            qCritical() << "CRITICAL: Failed to disable Double Buffering. Lines may look \"bended\" on your image.";
            qCritical() << "CRITICAL: Your graphics card or driver does not fully support Krita's OpenGL canvas.";
            qCritical() << "CRITICAL: For an optimal experience, please disable OpenGL";
            qCritical();
        }
#ifndef Q_WS_WIN
    }
#endif

    initializeCheckerShader();
    initializeDisplayShader();
}

void KisOpenGLCanvas2::resizeGL(int width, int height)
{
    glViewport(0, 0, (GLint)width, (GLint)height);
    coordinatesConverter()->setCanvasWidgetSize(QSize(width, height));
}

void KisOpenGLCanvas2::paintGL()
{
    makeCurrent();
    renderCanvasGL();
    renderDecorations();
}

void KisOpenGLCanvas2::drawCheckers()
{
    KisCoordinatesConverter *converter = coordinatesConverter();

    QTransform textureTransform;
    QTransform modelTransform;
    QRectF textureRect;
    QRectF modelRect;

    converter->getOpenGLCheckersInfo(&textureTransform, &modelTransform, &textureRect, &modelRect);

    // XXX: getting a config object every time we draw the checkers is bad for performance!
    KisConfig cfg;
    GLfloat checkSizeScale = KisOpenGLImageTextures::BACKGROUND_TEXTURE_CHECK_SIZE / static_cast<GLfloat>(cfg.checkSize());

    textureTransform *= QTransform::fromScale(checkSizeScale / KisOpenGLImageTextures::BACKGROUND_TEXTURE_SIZE,
                                              checkSizeScale / KisOpenGLImageTextures::BACKGROUND_TEXTURE_SIZE);

    d->checkerShader->bind();

    QMatrix4x4 projectionMatrix;
    projectionMatrix.setToIdentity();
    projectionMatrix.ortho(0, width(), height(), 0, NEAR_VAL, FAR_VAL);

    // Set view/projection matrices
    QMatrix4x4 modelMatrix(modelTransform);
    modelMatrix.optimize();
    modelMatrix = projectionMatrix * modelMatrix;
    d->checkerShader->setUniformValue("modelViewProjection", modelMatrix);

    QMatrix4x4 textureMatrix(textureTransform);
    d->checkerShader->setUniformValue("textureMatrix", textureMatrix);

    //Setup the geometry for rendering
    QVector<QVector3D> vertices;
    vertices << QVector3D(modelRect.left(),  modelRect.bottom(), 0.f)
             << QVector3D(modelRect.left(),  modelRect.top(),    0.f)
             << QVector3D(modelRect.right(), modelRect.bottom(), 0.f)
             << QVector3D(modelRect.left(),  modelRect.top(), 0.f)
             << QVector3D(modelRect.right(), modelRect.top(), 0.f)
             << QVector3D(modelRect.right(), modelRect.bottom(),    0.f);

    d->checkerShader->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    d->checkerShader->setAttributeArray(PROGRAM_VERTEX_ATTRIBUTE, vertices.constData());

    QVector<QVector2D> texCoords;
    texCoords << QVector2D(textureRect.left(), textureRect.bottom())
              << QVector2D(textureRect.left(), textureRect.top())
              << QVector2D(textureRect.right(), textureRect.bottom())
              << QVector2D(textureRect.left(), textureRect.top())
              << QVector2D(textureRect.right(), textureRect.top())
              << QVector2D(textureRect.right(), textureRect.bottom());

    d->checkerShader->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
    d->checkerShader->setAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE, texCoords.constData());

     // render checkers
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, d->openGLImageTextures->checkerTexture());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, 0);
    d->checkerShader->release();
}

void KisOpenGLCanvas2::drawImage()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    KisCoordinatesConverter *converter = coordinatesConverter();

    d->displayShader->bind();
    if (d->displayFilter) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, d->displayFilter->lutTexture());
    }

    QMatrix4x4 projectionMatrix;
    projectionMatrix.setToIdentity();
    projectionMatrix.ortho(0, width(), height(), 0, NEAR_VAL, FAR_VAL);

    // Set view/projection matrices
    QMatrix4x4 modelMatrix(coordinatesConverter()->imageToWidgetTransform());
    modelMatrix.optimize();
    modelMatrix = projectionMatrix * modelMatrix;
    d->displayShader->setUniformValue("modelViewProjection", modelMatrix);

    QMatrix4x4 textureMatrix;
    textureMatrix.setToIdentity();
    d->displayShader->setUniformValue("textureMatrix", textureMatrix);

    QRectF widgetRect(0,0, width(), height());
    QRectF widgetRectInImagePixels = converter->documentToImage(converter->widgetToDocument(widgetRect));

    qreal scaleX, scaleY;
    converter->imageScale(&scaleX, &scaleY);

    QRect wr = widgetRectInImagePixels.toAlignedRect() & d->openGLImageTextures->storedImageBounds();

    int firstColumn = d->openGLImageTextures->xToCol(wr.left());
    int lastColumn = d->openGLImageTextures->xToCol(wr.right());
    int firstRow = d->openGLImageTextures->yToRow(wr.top());
    int lastRow = d->openGLImageTextures->yToRow(wr.bottom());

    for (int col = firstColumn; col <= lastColumn; col++) {
        for (int row = firstRow; row <= lastRow; row++) {

            KisTextureTile *tile =
                    d->openGLImageTextures->getTextureTileCR(col, row);
            /*
             * We create a float rect here to workaround Qt's
             * "history reasons" in calculation of right()
             * and bottom() coordinates of integer rects.
             */
            QRectF textureRect(tile->tileRectInTexturePixels());
            QRectF modelRect(tile->tileRectInImagePixels());

            //Setup the geometry for rendering
            QVector<QVector3D> vertices;
            vertices << QVector3D(modelRect.left(),  modelRect.bottom(), 0.f)
                     << QVector3D(modelRect.left(),  modelRect.top(),    0.f)
                     << QVector3D(modelRect.right(), modelRect.bottom(), 0.f)
                     << QVector3D(modelRect.left(),  modelRect.top(), 0.f)
                     << QVector3D(modelRect.right(), modelRect.top(), 0.f)
                     << QVector3D(modelRect.right(), modelRect.bottom(),    0.f);

            d->displayShader->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
            d->displayShader->setAttributeArray(PROGRAM_VERTEX_ATTRIBUTE, vertices.constData());

            QVector<QVector2D> texCoords;
            texCoords << QVector2D(textureRect.left(), textureRect.bottom())
                      << QVector2D(textureRect.left(), textureRect.top())
                      << QVector2D(textureRect.right(), textureRect.bottom())
                      << QVector2D(textureRect.left(), textureRect.top())
                      << QVector2D(textureRect.right(), textureRect.top())
                      << QVector2D(textureRect.right(), textureRect.bottom());

            d->displayShader->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
            d->displayShader->setAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE, texCoords.constData());

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tile->textureId());

            if (SCALE_MORE_OR_EQUAL_TO(scaleX, scaleY, 2.0)) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }

            glDrawArrays(GL_TRIANGLES, 0, 6);

        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    d->displayShader->release();
}

void KisOpenGLCanvas2::initializeCheckerShader()
{
    delete d->checkerShader;
    d->checkerShader = new QGLShaderProgram();
    d->checkerShader->addShaderFromSourceFile(QGLShader::Vertex, KGlobal::dirs()->findResource("data", "krita/shaders/gl2.vert"));
    d->checkerShader->addShaderFromSourceFile(QGLShader::Fragment, KGlobal::dirs()->findResource("data", "krita/shaders/checker.frag"));
    d->checkerShader->bindAttributeLocation("a_vertexPosition", PROGRAM_VERTEX_ATTRIBUTE);
    d->checkerShader->bindAttributeLocation("a_textureCoordinate", PROGRAM_TEXCOORD_ATTRIBUTE);

    if (! d->checkerShader->link()) {
        qDebug() << "OpenGL error" << glGetError();
        qFatal("Failed linking checker shader");
    }
    Q_ASSERT(d->checkerShader->isLinked());
}

void KisOpenGLCanvas2::initializeDisplayShader()
{
    delete d->displayShader;
    d->displayShader = new QGLShaderProgram();

    d->displayShader->addShaderFromSourceFile(QGLShader::Vertex, KGlobal::dirs()->findResource("data", "krita/shaders/gl2.vert"));
    if (d->displayFilter) {
        d->displayShader->addShaderFromSourceCode(QGLShader::Fragment, d->displayFilter->program().toLatin1());
    }
    else {
        d->displayShader->addShaderFromSourceFile(QGLShader::Fragment, KGlobal::dirs()->findResource("data", "krita/shaders/display.frag"));
    }
    d->displayShader->bindAttributeLocation("a_vertexPosition", PROGRAM_VERTEX_ATTRIBUTE);
    d->displayShader->bindAttributeLocation("a_textureCoordinate", PROGRAM_TEXCOORD_ATTRIBUTE);

    if (! d->displayShader->link()) {
        qDebug() << "OpenGL error" << glGetError();
        qFatal("Failed linking display shader");
    }

    Q_ASSERT(d->displayShader->isLinked());
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

void KisOpenGLCanvas2::renderCanvasGL()
{
    // Draw the border (that is, clear the whole widget to the border color)
    QColor widgetBackgroundColor = borderColor();
    glClearColor(widgetBackgroundColor.redF(), widgetBackgroundColor.greenF(), widgetBackgroundColor.blueF(), 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    drawCheckers();
    drawImage();
}

void KisOpenGLCanvas2::renderDecorations()
{
    QPainter gc(this);
    QRect boundingRect = coordinatesConverter()->imageRectInWidgetPixels().toAlignedRect();
    drawDecorations(gc, boundingRect);
    gc.end();
}

bool KisOpenGLCanvas2::callFocusNextPrevChild(bool next)
{
    return focusNextPrevChild(next);
}

#include "kis_opengl_canvas2.moc"
#endif // HAVE_OPENGL
