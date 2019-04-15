/*
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
 * Copyright (C) Michael Abrahams <miabraha@gmail.com>, (C) 2015
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

#ifndef KIS_OPENGL_CANVAS_2_H
#define KIS_OPENGL_CANVAS_2_H

#include <QOpenGLWidget>
#ifndef Q_OS_OSX
#include <QOpenGLFunctions>
#else
#include <QOpenGLFunctions_3_2_Core>
#endif
#include "canvas/kis_canvas_widget_base.h"
#include "opengl/kis_opengl_image_textures.h"

#include "kritaui_export.h"
#include "kis_ui_types.h"

class KisCanvas2;
class KisDisplayColorConverter;
class QOpenGLShaderProgram;
class QPainterPath;

#ifndef Q_MOC_RUN
#ifndef Q_OS_OSX
#define GLFunctions QOpenGLFunctions
#else
#define GLFunctions QOpenGLFunctions_3_2_Core
#endif

#endif
/**
 * KisOpenGLCanvas is the widget that shows the actual image using OpenGL
 *
 * NOTE: if you change something in the event handling here, also change it
 * in the qpainter canvas.
 *
 */
class KRITAUI_EXPORT KisOpenGLCanvas2
        : public QOpenGLWidget
#ifndef Q_MOC_RUN
        , protected GLFunctions
#endif
        , public KisCanvasWidgetBase
{
    Q_OBJECT

public:

    KisOpenGLCanvas2(KisCanvas2 *canvas, KisCoordinatesConverter *coordinatesConverter, QWidget *parent, KisImageWSP image, KisDisplayColorConverter *colorConverter);

    ~KisOpenGLCanvas2() override;

public: // QOpenGLWidget

    void resizeGL(int width, int height) override;
    void initializeGL() override;
    void paintGL() override;

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
    void inputMethodEvent(QInputMethodEvent *event) override;

public:
    void renderCanvasGL();
    void renderDecorations(QPainter *painter);
    void paintToolOutline(const QPainterPath &path);

public: // Implement kis_abstract_canvas_widget interface
    void setDisplayFilter(QSharedPointer<KisDisplayFilter> displayFilter) override;
    void notifyImageColorSpaceChanged(const KoColorSpace *cs) override;

    void setWrapAroundViewingMode(bool value) override;
    void channelSelectionChanged(const QBitArray &channelFlags) override;
    void setDisplayColorConverter(KisDisplayColorConverter *colorConverter) override;
    void finishResizingImage(qint32 w, qint32 h) override;
    KisUpdateInfoSP startUpdateCanvasProjection(const QRect & rc, const QBitArray &channelFlags) override;
    QRect updateCanvasProjection(KisUpdateInfoSP info) override;
    QVector<QRect> updateCanvasProjection(const QVector<KisUpdateInfoSP> &infoObjects) override;

    QWidget *widget() override {
        return this;
    }

    bool isBusy() const override;
    void setLodResetInProgress(bool value) override;

    void setDisplayFilterImpl(QSharedPointer<KisDisplayFilter> displayFilter, bool initializing);

    KisOpenGLImageTexturesSP openGLImageTextures() const;

public Q_SLOTS:
    void slotConfigChanged();
    void slotPixelGridModeChanged();

protected: // KisCanvasWidgetBase
    bool callFocusNextPrevChild(bool next) override;

private:
    void initializeShaders();
    void initializeDisplayShader();

    void reportFailedShaderCompilation(const QString &context);
    void drawImage();
    void drawCheckers();
    void drawGrid();
    QSize viewportDevicePixelSize() const;
    QSizeF widgetSizeAlignedToDevicePixel() const;

private:

    struct Private;
    Private * const d;

};

#endif // KIS_OPENGL_CANVAS_2_H
