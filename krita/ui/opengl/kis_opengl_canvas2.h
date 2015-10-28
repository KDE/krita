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

#include <opengl/kis_opengl.h>

#ifdef HAVE_OPENGL

#include <QOpenGLWidget>
#include <QOpenGLFunctions>

#include <KoCanvasBase.h>

#include "canvas/kis_canvas_widget_base.h"
#include "opengl/kis_opengl_image_textures.h"

#include "kritaui_export.h"
#include "kis_ui_types.h"

class QWidget;
class KisCanvas2;
class KisDisplayColorConverter;
class QOpenGLShaderProgram;
class QPainterPath;


/**
 * KisOpenGLCanvas is the widget that shows the actual image using OpenGL
 *
 * NOTE: if you change something in the event handling here, also change it
 * in the qpainter canvas.
 *
 */
class KRITAUI_EXPORT KisOpenGLCanvas2 : public QOpenGLWidget, public QOpenGLFunctions, public KisCanvasWidgetBase
{

    Q_OBJECT

public:

    KisOpenGLCanvas2(KisCanvas2 *canvas, KisCoordinatesConverter *coordinatesConverter, QWidget *parent, KisImageWSP image, KisDisplayColorConverter *colorConverter);

    virtual ~KisOpenGLCanvas2();

public: // QOpenGLWidget

    void resizeGL(int width, int height);
    void initializeGL();
    void paintGL();

    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
    virtual void inputMethodEvent(QInputMethodEvent *event);

public:
    void initializeCheckerShader();
    void initializeDisplayShader();
    void renderCanvasGL();
    void renderDecorations(QPainter *painter);
    void paintToolOutline(const QPainterPath &path);


public: // Implement kis_abstract_canvas_widget interface
    void setDisplayFilter(KisDisplayFilter* displayFilter);
    void setWrapAroundViewingMode(bool value);
    void channelSelectionChanged(const QBitArray &channelFlags);
    void setDisplayProfile(KisDisplayColorConverter *colorConverter);
    void disconnectCurrentCanvas();
    void finishResizingImage(qint32 w, qint32 h);
    KisUpdateInfoSP startUpdateCanvasProjection(const QRect & rc, const QBitArray &channelFlags);
    QRect updateCanvasProjection(KisUpdateInfoSP info);

    QWidget *widget() {
        return this;
    }

    bool isBusy() const;

    void setDisplayFilterImpl(KisDisplayFilter* displayFilter, bool initializing);

    KisOpenGLImageTexturesSP openGLImageTextures() const;

private Q_SLOTS:
    void slotConfigChanged();

protected: // KisCanvasWidgetBase
    virtual bool callFocusNextPrevChild(bool next);

private:
    void reportShaderLinkFailedAndExit(bool result, const QString &context, const QString &log);
    QOpenGLShaderProgram *getCursorShader();
    void drawImage();
    void drawCheckers();
    QByteArray buildFragmentShader();

private:

    struct Private;
    Private * const d;

};

#endif // HAVE_OPENGL
#endif // KIS_OPENGL_CANVAS_2_H
