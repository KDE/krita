/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006-2013
 * Copyright (C) 2015 Michael Abrahams <miabraha@gmail.com>
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

#define GL_GLEXT_PROTOTYPES

#include "opengl/kis_opengl_canvas2.h"
#include "opengl/kis_opengl_canvas2_p.h"

#include "opengl/kis_opengl_canvas_debugger.h"
#include "canvas/kis_canvas2.h"
#include "canvas/kis_coordinates_converter.h"
#include "canvas/kis_display_filter.h"
#include "canvas/kis_display_color_converter.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_debug.h"

#include <QPainter>
#include <QPainterPath>
#include <QPointF>
#include <QMatrix>
#include <QTransform>
#include <QThread>
#include <QFile>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMessageBox>

#define NEAR_VAL -1000.0
#define FAR_VAL 1000.0

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

static bool OPENGL_SUCCESS = false;

typedef void (*kis_glLogicOp)(int);
static kis_glLogicOp ptr_glLogicOp = 0;


struct KisOpenGLCanvas2::Private
{
public:

    ~Private() {
        delete displayShader;
        delete checkerShader;
        delete cursorShader;
        Sync::deleteSync(glSyncObject);
    }

    bool canvasInitialized{false};

    KisOpenGLImageTexturesSP openGLImageTextures;

    QOpenGLShaderProgram *displayShader{0};
    int displayUniformLocationModelViewProjection;
    int displayUniformLocationTextureMatrix;
    int displayUniformLocationViewPortScale;
    int displayUniformLocationTexelSize;
    int displayUniformLocationTexture0;
    int displayUniformLocationTexture1;
    int displayUniformLocationFixedLodLevel;

    QOpenGLShaderProgram *checkerShader{0};
    GLfloat checkSizeScale;
    bool scrollCheckers;
    int checkerUniformLocationModelViewProjection;
    int checkerUniformLocationTextureMatrix;

    QOpenGLShaderProgram *cursorShader{0};
    int cursorShaderModelViewProjectionUniform;

    KisDisplayFilter* displayFilter;
    KisOpenGL::FilterMode filterMode;
    bool proofingConfigIsUpdated=false;

    GLsync glSyncObject{0};

    bool wrapAroundMode{false};

    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer buffers[3];

    QVector3D vertices[6];
    QVector2D texCoords[6];

    int xToColWithWrapCompensation(int x, const QRect &imageRect) {
        int firstImageColumn = openGLImageTextures->xToCol(imageRect.left());
        int lastImageColumn = openGLImageTextures->xToCol(imageRect.right());

        int colsPerImage = lastImageColumn - firstImageColumn + 1;
        int numWraps = floor(qreal(x) / imageRect.width());
        int remainder = x - imageRect.width() * numWraps;

        return colsPerImage * numWraps + openGLImageTextures->xToCol(remainder);
    }

    int yToRowWithWrapCompensation(int y, const QRect &imageRect) {
        int firstImageRow = openGLImageTextures->yToRow(imageRect.top());
        int lastImageRow = openGLImageTextures->yToRow(imageRect.bottom());

        int rowsPerImage = lastImageRow - firstImageRow + 1;
        int numWraps = floor(qreal(y) / imageRect.height());
        int remainder = y - imageRect.height() * numWraps;

        return rowsPerImage * numWraps + openGLImageTextures->yToRow(remainder);
    }

};

KisOpenGLCanvas2::KisOpenGLCanvas2(KisCanvas2 *canvas,
                                   KisCoordinatesConverter *coordinatesConverter,
                                   QWidget *parent,
                                   KisImageWSP image,
                                   KisDisplayColorConverter *colorConverter)
    : QOpenGLWidget(parent)
    , KisCanvasWidgetBase(canvas, coordinatesConverter)
    , d(new Private())
{
    KisConfig cfg;
    cfg.writeEntry("canvasState", "OPENGL_STARTED");

    d->openGLImageTextures =
            KisOpenGLImageTextures::getImageTextures(image,
                                                     colorConverter->monitorProfile(),
                                                     colorConverter->renderingIntent(),
                                                     colorConverter->conversionFlags());

    setAcceptDrops(true);
    setAutoFillBackground(false);

    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_NoSystemBackground, true);
#ifdef Q_OS_MAC
    setAttribute(Qt::WA_AcceptTouchEvents, false);
#else
    setAttribute(Qt::WA_AcceptTouchEvents, true);
#endif
    setAttribute(Qt::WA_InputMethodEnabled, true);
    setAttribute(Qt::WA_DontCreateNativeAncestors, true);

    setDisplayFilterImpl(colorConverter->displayFilter(), true);

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    slotConfigChanged();
    cfg.writeEntry("canvasState", "OPENGL_SUCCESS");
}

KisOpenGLCanvas2::~KisOpenGLCanvas2()
{
    delete d;
}

bool KisOpenGLCanvas2::needsFpsDebugging() const
{
    return KisOpenglCanvasDebugger::instance()->showFpsOnCanvas();
}

void KisOpenGLCanvas2::setDisplayFilter(KisDisplayFilter* displayFilter) {
    setDisplayFilterImpl(displayFilter, false);
}

void KisOpenGLCanvas2::setDisplayFilterImpl(KisDisplayFilter* displayFilter, bool initializing)
{
    bool needsInternalColorManagement =
            !displayFilter || displayFilter->useInternalColorManagement();

    bool needsFullRefresh = d->openGLImageTextures->setInternalColorManagementActive(needsInternalColorManagement);

    d->displayFilter = displayFilter;
    if (d->canvasInitialized) {
        d->canvasInitialized = false;
        initializeDisplayShader();
        initializeCheckerShader();
        d->canvasInitialized = true;
    }

    if (!initializing && needsFullRefresh) {
        canvas()->startUpdateInPatches(canvas()->image()->bounds());
    }
    else if (!initializing)  {
        canvas()->updateCanvas();
    }
}

void KisOpenGLCanvas2::setWrapAroundViewingMode(bool value)
{
    d->wrapAroundMode = value;
    update();
}

inline void rectToVertices(QVector3D* vertices, const QRectF &rc)
{
    vertices[0] = QVector3D(rc.left(),  rc.bottom(), 0.f);
    vertices[1] = QVector3D(rc.left(),  rc.top(),    0.f);
    vertices[2] = QVector3D(rc.right(), rc.bottom(), 0.f);
    vertices[3] = QVector3D(rc.left(),  rc.top(), 0.f);
    vertices[4] = QVector3D(rc.right(), rc.top(), 0.f);
    vertices[5] = QVector3D(rc.right(), rc.bottom(),    0.f);
}

inline void rectToTexCoords(QVector2D* texCoords, const QRectF &rc)
{
    texCoords[0] = QVector2D(rc.left(), rc.bottom());
    texCoords[1] = QVector2D(rc.left(), rc.top());
    texCoords[2] = QVector2D(rc.right(), rc.bottom());
    texCoords[3] = QVector2D(rc.left(), rc.top());
    texCoords[4] = QVector2D(rc.right(), rc.top());
    texCoords[5] = QVector2D(rc.right(), rc.bottom());
}

void KisOpenGLCanvas2::initializeGL()
{
    KisOpenGL::initializeContext(context());
    initializeOpenGLFunctions();

    KisConfig cfg;
    d->openGLImageTextures->setProofingConfig(canvas()->proofingConfiguration());
    d->openGLImageTextures->initGL(context()->functions());
    d->openGLImageTextures->generateCheckerTexture(createCheckersImage(cfg.checkSize()));
    initializeCheckerShader();
    initializeDisplayShader();

    d->vao.create();
    d->vao.bind();
    glEnableVertexAttribArray(PROGRAM_VERTEX_ATTRIBUTE);
    glEnableVertexAttribArray(PROGRAM_TEXCOORD_ATTRIBUTE);

    d->buffers[0].create();
    d->buffers[0].setUsagePattern(QOpenGLBuffer::StaticDraw);
    d->buffers[1].create();
    d->buffers[2].setUsagePattern(QOpenGLBuffer::StaticDraw);
    d->buffers[2].create();
    d->buffers[2].setUsagePattern(QOpenGLBuffer::StreamDraw);

    d->buffers[0].bind();
    d->buffers[0].allocate(d->vertices, 6 * 3 * sizeof(float));
    glVertexAttribPointer(PROGRAM_VERTEX_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 0, 0);

    d->buffers[1].bind();
    d->buffers[1].allocate(d->texCoords, 6 * 2 * sizeof(float));
    glVertexAttribPointer(PROGRAM_TEXCOORD_ATTRIBUTE, 2, GL_FLOAT, GL_FALSE, 0, 0);

    ptr_glLogicOp = (kis_glLogicOp)(context()->getProcAddress("glLogicOp"));

    Sync::init(context());

    d->canvasInitialized = true;
}

void KisOpenGLCanvas2::resizeGL(int width, int height)
{
    coordinatesConverter()->setCanvasWidgetSize(QSize(width, height));
    paintGL();
}

void KisOpenGLCanvas2::paintGL()
{
    if (!OPENGL_SUCCESS) {
        KisConfig cfg;
        cfg.writeEntry("canvasState", "OPENGL_PAINT_STARTED");
    }

    KisOpenglCanvasDebugger::instance()->nofityPaintRequested();

    d->vao.bind();
    renderCanvasGL();
    d->vao.release();

    if (d->glSyncObject) {
        Sync::deleteSync(d->glSyncObject);
    }
    d->glSyncObject = Sync::getSync();

    qDebug() << "DRAWING DECORATIONS";
    QPainter gc(this);
    renderDecorations(&gc);
    // QPen pen;
    // pen.setWidth(10);
    // pen.setBrush(Qt::green);
    //
    // pen.setJoinStyle(Qt::RoundJoin);
    // gc.setPen(pen);
    // gc.drawRect(100, 100, 100, 100);
    //
    // pen.setJoinStyle(Qt::BevelJoin);
    // gc.setPen(pen);
    // gc.drawRect(210, 100, 100, 100);
    //
    // QBrush brush(Qt::red);
    // gc.fillRect(320, 100, 100, 100, brush);
    gc.end();

    if (!OPENGL_SUCCESS) {
        KisConfig cfg;
        cfg.writeEntry("canvasState", "OPENGL_SUCCESS");
        OPENGL_SUCCESS = true;
    }
}

QOpenGLShaderProgram *KisOpenGLCanvas2::getCursorShader()
{
    if (d->cursorShader == 0) {
        d->cursorShader = new QOpenGLShaderProgram();
        d->cursorShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/cursor.vert");
        d->cursorShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/cursor.frag");
        d->cursorShader->bindAttributeLocation("a_vertexPosition", PROGRAM_VERTEX_ATTRIBUTE);
        if (! d->cursorShader->link()) {
            dbgUI << "OpenGL error" << glGetError();
            qFatal("Failed linking cursor shader");
        }
        Q_ASSERT(d->cursorShader->isLinked());
        d->cursorShaderModelViewProjectionUniform = d->cursorShader->uniformLocation("modelViewProjection");
    }

    return d->cursorShader;
}

void KisOpenGLCanvas2::paintToolOutline(const QPainterPath &path)
{
    QOpenGLShaderProgram *cursorShader = getCursorShader();
    cursorShader->bind();

    // setup the mvp transformation
    KisCoordinatesConverter *converter = coordinatesConverter();

    QMatrix4x4 projectionMatrix;
    projectionMatrix.setToIdentity();
    projectionMatrix.ortho(0, width(), height(), 0, NEAR_VAL, FAR_VAL);

    // Set view/projection matrices
    QMatrix4x4 modelMatrix(converter->flakeToWidgetTransform());
    modelMatrix.optimize();
    modelMatrix = projectionMatrix * modelMatrix;
    cursorShader->setUniformValue(d->cursorShaderModelViewProjectionUniform, modelMatrix);

    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    // XXX: glLogicOp not in ES 2.0 -- it would be better to use another method.
    // It is defined in 3.1 core profile onward.
    // Actually, https://www.opengl.org/sdk/docs/man/html/glLogicOp.xhtml says it's in 2.0 onwards,
    // only not in ES, but we don't care about ES, so we could use the function directly.
    glEnable(GL_COLOR_LOGIC_OP);
    if (ptr_glLogicOp) {
        ptr_glLogicOp(GL_XOR);
    }

    // Paint the tool outline
    d->buffers[2].bind();

    // Convert every disjointed subpath to a polygon and draw that polygon
    QList<QPolygonF> subPathPolygons = path.toSubpathPolygons();
    for (int i = 0; i < subPathPolygons.size(); i++) {
        const QPolygonF& polygon = subPathPolygons.at(i);

        QVector<QVector3D> vertices;
        vertices.resize(polygon.count());
        for (int j = 0; j < polygon.count(); j++) {
            QPointF p = polygon.at(j);
            vertices[j].setX(p.x());
            vertices[j].setY(p.y());
        }

        d->buffers[2].allocate(vertices.constData(), 3 * vertices.size() * sizeof(float));

        glDrawArrays(GL_LINE_STRIP, 0, vertices.size());
    }

    d->buffers[2].release();

    glDisable(GL_COLOR_LOGIC_OP);

    cursorShader->release();
}

bool KisOpenGLCanvas2::isBusy() const
{
    const bool isBusyStatus = Sync::syncStatus(d->glSyncObject) == Sync::Unsignaled;
    KisOpenglCanvasDebugger::instance()->nofitySyncStatus(isBusyStatus);

    return isBusyStatus;
}

void KisOpenGLCanvas2::drawCheckers()
{
    if (!d->checkerShader) {
        return;
    }

    KisCoordinatesConverter *converter = coordinatesConverter();
    QTransform textureTransform;
    QTransform modelTransform;
    QRectF textureRect;
    QRectF modelRect;

    QRectF viewportRect = !d->wrapAroundMode ?
                converter->imageRectInViewportPixels() :
                converter->widgetToViewport(this->rect());

    converter->getOpenGLCheckersInfo(viewportRect,
                                     &textureTransform, &modelTransform, &textureRect, &modelRect, d->scrollCheckers);

    textureTransform *= QTransform::fromScale(d->checkSizeScale / KisOpenGLImageTextures::BACKGROUND_TEXTURE_SIZE,
                                              d->checkSizeScale / KisOpenGLImageTextures::BACKGROUND_TEXTURE_SIZE);

    if (!d->checkerShader->bind()) {
        qWarning() << "Could not bind checker shader";
        return;
    }

    QMatrix4x4 projectionMatrix;
    projectionMatrix.setToIdentity();
    projectionMatrix.ortho(0, width(), height(), 0, NEAR_VAL, FAR_VAL);

    // Set view/projection matrices
    QMatrix4x4 modelMatrix(modelTransform);
    modelMatrix.optimize();
    modelMatrix = projectionMatrix * modelMatrix;

    d->checkerShader->setUniformValue(d->checkerUniformLocationModelViewProjection, modelMatrix);

    QMatrix4x4 textureMatrix(textureTransform);
    d->checkerShader->setUniformValue(d->checkerUniformLocationTextureMatrix, textureMatrix);

    //Setup the geometry for rendering
    rectToVertices(d->vertices, modelRect);

    d->buffers[0].bind();
    d->buffers[0].write(0, d->vertices, 3 * 6 * sizeof(float));

    rectToTexCoords(d->texCoords, textureRect);
    d->buffers[1].bind();
    d->buffers[2].write(0, d->texCoords, 2 * 6 * sizeof(float));

    // render checkers
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, d->openGLImageTextures->checkerTexture());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, 0);
    d->checkerShader->release();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void KisOpenGLCanvas2::drawImage()
{
    if (!d->displayShader) {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    KisCoordinatesConverter *converter = coordinatesConverter();

    d->displayShader->bind();

    QMatrix4x4 projectionMatrix;
    projectionMatrix.setToIdentity();
    projectionMatrix.ortho(0, width(), height(), 0, NEAR_VAL, FAR_VAL);

    // Set view/projection matrices
    QMatrix4x4 modelMatrix(coordinatesConverter()->imageToWidgetTransform());
    modelMatrix.optimize();
    modelMatrix = projectionMatrix * modelMatrix;
    d->displayShader->setUniformValue(d->displayUniformLocationModelViewProjection, modelMatrix);

    QMatrix4x4 textureMatrix;
    textureMatrix.setToIdentity();
    d->displayShader->setUniformValue(d->displayUniformLocationTextureMatrix, textureMatrix);

    QRectF widgetRect(0,0, width(), height());
    QRectF widgetRectInImagePixels = converter->documentToImage(converter->widgetToDocument(widgetRect));

    qreal scaleX, scaleY;
    converter->imageScale(&scaleX, &scaleY);
    d->displayShader->setUniformValue(d->displayUniformLocationViewPortScale, (GLfloat) scaleX);
    d->displayShader->setUniformValue(d->displayUniformLocationTexelSize, (GLfloat) d->openGLImageTextures->texelSize());

    QRect ir = d->openGLImageTextures->storedImageBounds();
    QRect wr = widgetRectInImagePixels.toAlignedRect();

    if (!d->wrapAroundMode) {
        // if we don't want to paint wrapping images, just limit the
        // processing area, and the code will handle all the rest
        wr &= ir;
    }

    int firstColumn = d->xToColWithWrapCompensation(wr.left(), ir);
    int lastColumn = d->xToColWithWrapCompensation(wr.right(), ir);
    int firstRow = d->yToRowWithWrapCompensation(wr.top(), ir);
    int lastRow = d->yToRowWithWrapCompensation(wr.bottom(), ir);

    int minColumn = d->openGLImageTextures->xToCol(ir.left());
    int maxColumn = d->openGLImageTextures->xToCol(ir.right());
    int minRow = d->openGLImageTextures->yToRow(ir.top());
    int maxRow = d->openGLImageTextures->yToRow(ir.bottom());

    int imageColumns = maxColumn - minColumn + 1;
    int imageRows = maxRow - minRow + 1;

    for (int col = firstColumn; col <= lastColumn; col++) {
        for (int row = firstRow; row <= lastRow; row++) {

            int effectiveCol = col;
            int effectiveRow = row;
            QPointF tileWrappingTranslation;

            if (effectiveCol > maxColumn || effectiveCol < minColumn) {
                int translationStep = floor(qreal(col) / imageColumns);
                int originCol = translationStep * imageColumns;
                effectiveCol = col - originCol;
                tileWrappingTranslation.rx() = translationStep * ir.width();
            }

            if (effectiveRow > maxRow || effectiveRow < minRow) {
                int translationStep = floor(qreal(row) / imageRows);
                int originRow = translationStep * imageRows;
                effectiveRow = row - originRow;
                tileWrappingTranslation.ry() = translationStep * ir.height();
            }

            KisTextureTile *tile =
                    d->openGLImageTextures->getTextureTileCR(effectiveCol, effectiveRow);

            if (!tile) {
                warnUI << "OpenGL: Trying to paint texture tile but it has not been created yet.";
                continue;
            }

            /*
             * We create a float rect here to workaround Qt's
             * "history reasons" in calculation of right()
             * and bottom() coordinates of integer rects.
             */
            QRectF textureRect(tile->tileRectInTexturePixels());
            QRectF modelRect(tile->tileRectInImagePixels().translated(tileWrappingTranslation.x(), tileWrappingTranslation.y()));

            //Setup the geometry for rendering
            rectToVertices(d->vertices, modelRect);

            d->buffers[0].bind();
            d->buffers[0].write(0, d->vertices, 3 * 6 * sizeof(float));

            rectToTexCoords(d->texCoords, textureRect);
            d->buffers[1].bind();
            d->buffers[1].write(0, d->texCoords, 2 * 6 * sizeof(float));

            if (d->displayFilter) {
                glActiveTexture(GL_TEXTURE0 + 1);
                glBindTexture(GL_TEXTURE_3D, d->displayFilter->lutTexture());
                d->displayShader->setUniformValue(d->displayUniformLocationTexture1, 1);
            }

            int currentLodPlane = tile->currentLodPlane();
            if (d->displayUniformLocationFixedLodLevel >= 0) {
                d->displayShader->setUniformValue(d->displayUniformLocationFixedLodLevel,
                                                  (GLfloat) currentLodPlane);
            }

            glActiveTexture(GL_TEXTURE0);
            tile->bindToActiveTexture();

            if (currentLodPlane) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            } else if (SCALE_MORE_OR_EQUAL_TO(scaleX, scaleY, 2.0)) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                switch(d->filterMode) {
                case KisOpenGL::NearestFilterMode:
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    break;
                case KisOpenGL::BilinearFilterMode:
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    break;
                case KisOpenGL::TrilinearFilterMode:
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    break;
                case KisOpenGL::HighQualityFiltering:
                    if (SCALE_LESS_THAN(scaleX, scaleY, 0.5)) {
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                    } else {
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    }
                    break;
                }
            }

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    d->displayShader->release();
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void KisOpenGLCanvas2::reportShaderLinkFailedAndExit(bool result, const QString &context, const QString &log)
{
    KisConfig cfg;

    if (cfg.useVerboseOpenGLDebugOutput()) {
        dbgUI << "GL-log:" << context << log;
    }

    if (result) return;

    QMessageBox::critical(this, i18nc("@title:window", "Krita"),
                          QString(i18n("Krita could not initialize the OpenGL canvas:\n\n%1\n\n%2\n\n Krita will disable OpenGL and close now.")).arg(context).arg(log),
                          QMessageBox::Close);

    cfg.setUseOpenGL(false);
    cfg.setCanvasState("OPENGL_FAILED");
}

void KisOpenGLCanvas2::initializeCheckerShader()
{
    if (d->canvasInitialized) return;

    delete d->checkerShader;
    d->checkerShader = new QOpenGLShaderProgram();
    QString vertexShaderName;
    QString fragmentShaderName;

    bool result;

    if (KisOpenGL::hasOpenGL3()) {
        vertexShaderName = ":/matrix_transform.vert";
        fragmentShaderName = ":/simple_texture.frag";
    }
    else {
        vertexShaderName = ":/matrix_transform_legacy.vert";
        fragmentShaderName = ":/simple_texture_legacy.frag";
    }

    result = d->checkerShader->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShaderName);
    reportShaderLinkFailedAndExit(result, "Checker vertex shader", d->checkerShader->log());

    result = d->checkerShader->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShaderName);
    reportShaderLinkFailedAndExit(result, "Checker fragment shader", d->checkerShader->log());


    d->checkerShader->bindAttributeLocation("a_vertexPosition", PROGRAM_VERTEX_ATTRIBUTE);
    d->checkerShader->bindAttributeLocation("a_textureCoordinate", PROGRAM_TEXCOORD_ATTRIBUTE);

    result = d->checkerShader->link();
    reportShaderLinkFailedAndExit(result, "Checker shader (link)", d->checkerShader->log());

    Q_ASSERT(d->checkerShader->isLinked());

    d->checkerUniformLocationModelViewProjection = d->checkerShader->uniformLocation("modelViewProjection");
    d->checkerUniformLocationTextureMatrix = d->checkerShader->uniformLocation("textureMatrix");

}

QByteArray KisOpenGLCanvas2::buildFragmentShader()
{
    QByteArray shaderText;

    bool haveDisplayFilter = d->displayFilter && !d->displayFilter->program().isEmpty();
    bool useHiQualityFiltering = d->filterMode == KisOpenGL::HighQualityFiltering;

    QString filename;

    if (KisOpenGL::hasOpenGL3()) {
        filename = "highq_downscale.frag";
        shaderText.append("#version 150\n");
    } else {
        filename = "simple_texture_legacy.frag";
    }

    if (haveDisplayFilter) {
        shaderText.append("#define USE_OCIO\n");
        shaderText.append(d->displayFilter->program().toLatin1());
    }

    if (KisOpenGL::hasOpenGL3() && useHiQualityFiltering) {
        shaderText.append("#define HIGHQ_SCALING\n");
    }
    if (KisOpenGL::hasOpenGL3()) {
        shaderText.append("#define DIRECT_LOD_FETCH\n");
    }

    {
        QFile prefaceFile(":/" + filename);
        prefaceFile.open(QIODevice::ReadOnly);
        shaderText.append(prefaceFile.readAll());
    }

    return shaderText;
}

void KisOpenGLCanvas2::initializeDisplayShader()
{
    if (d->canvasInitialized) return;

    delete d->displayShader;
    d->displayShader = new QOpenGLShaderProgram();

    bool result = d->displayShader->addShaderFromSourceCode(QOpenGLShader::Fragment, buildFragmentShader());
    reportShaderLinkFailedAndExit(result, "Display fragment shader", d->displayShader->log());

    if (KisOpenGL::hasOpenGL3()) {
        result = d->displayShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/matrix_transform.vert");
    }
    else {
        result = d->displayShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/matrix_transform_legacy.vert");
    }

    reportShaderLinkFailedAndExit(result, "Display vertex shader", d->displayShader->log());

    d->displayShader->bindAttributeLocation("a_vertexPosition", PROGRAM_VERTEX_ATTRIBUTE);
    d->displayShader->bindAttributeLocation("a_textureCoordinate", PROGRAM_TEXCOORD_ATTRIBUTE);

    result = d->displayShader->link();
    reportShaderLinkFailedAndExit(result, "Display shader (link)", d->displayShader->log());

    Q_ASSERT(d->displayShader->isLinked());

    d->displayUniformLocationModelViewProjection = d->displayShader->uniformLocation("modelViewProjection");
    d->displayUniformLocationTextureMatrix = d->displayShader->uniformLocation("textureMatrix");
    d->displayUniformLocationTexture0 = d->displayShader->uniformLocation("texture0");

    // ocio
    d->displayUniformLocationTexture1 = d->displayShader->uniformLocation("texture1");

    // highq || lod
    d->displayUniformLocationViewPortScale = d->displayShader->uniformLocation("viewportScale");

    // highq
    d->displayUniformLocationTexelSize = d->displayShader->uniformLocation("texelSize");

    // TODO: The trilinear filtering mode is having issues when that is set in the application. It sometimes causes Krita to crash
    // I cannot tell where that is at in here... Scott P (11/8/2015)

    // lod
    d->displayUniformLocationFixedLodLevel =
            KisOpenGL::hasOpenGL3() ?
                d->displayShader->uniformLocation("fixedLodLevel") : -1;
}

void KisOpenGLCanvas2::slotConfigChanged()
{
    KisConfig cfg;
    d->checkSizeScale = KisOpenGLImageTextures::BACKGROUND_TEXTURE_CHECK_SIZE / static_cast<GLfloat>(cfg.checkSize());
    d->scrollCheckers = cfg.scrollCheckers();

    d->openGLImageTextures->generateCheckerTexture(createCheckersImage(cfg.checkSize()));
    d->openGLImageTextures->updateConfig(cfg.useOpenGLTextureBuffer(), cfg.numMipmapLevels());
    d->filterMode = (KisOpenGL::FilterMode) cfg.openGLFilteringMode();

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

    if (d->displayFilter) {
        d->displayFilter->updateShader();
    }

    drawCheckers();
    drawImage();
}

void KisOpenGLCanvas2::renderDecorations(QPainter *painter)
{
    QRect boundingRect = coordinatesConverter()->imageRectInWidgetPixels().toAlignedRect();
    drawDecorations(*painter, boundingRect);
}


void KisOpenGLCanvas2::setDisplayProfile(KisDisplayColorConverter *colorConverter)
{
    d->openGLImageTextures->setMonitorProfile(colorConverter->monitorProfile(),
                                              colorConverter->renderingIntent(),
                                              colorConverter->conversionFlags());
}

void KisOpenGLCanvas2::channelSelectionChanged(const QBitArray &channelFlags)
{
    d->openGLImageTextures->setChannelFlags(channelFlags);
}


void KisOpenGLCanvas2::finishResizingImage(qint32 w, qint32 h)
{
    if (d->canvasInitialized) {
        d->openGLImageTextures->slotImageSizeChanged(w, h);
    }
}

KisUpdateInfoSP KisOpenGLCanvas2::startUpdateCanvasProjection(const QRect & rc, const QBitArray &channelFlags)
{
    d->openGLImageTextures->setChannelFlags(channelFlags);
    if (canvas()->proofingConfigUpdated()) {
        d->openGLImageTextures->setProofingConfig(canvas()->proofingConfiguration());
        canvas()->setProofingConfigUpdated(false);
    }
    return d->openGLImageTextures->updateCache(rc);
}


QRect KisOpenGLCanvas2::updateCanvasProjection(KisUpdateInfoSP info)
{
    // See KisQPainterCanvas::updateCanvasProjection for more info
    bool isOpenGLUpdateInfo = dynamic_cast<KisOpenGLUpdateInfo*>(info.data());
    if (isOpenGLUpdateInfo) {
        d->openGLImageTextures->recalculateCache(info);
    }
    return QRect(); // FIXME: Implement dirty rect for OpenGL
}

bool KisOpenGLCanvas2::callFocusNextPrevChild(bool next)
{
    return focusNextPrevChild(next);
}

KisOpenGLImageTexturesSP KisOpenGLCanvas2::openGLImageTextures() const
{
    return d->openGLImageTextures;
}
