    /*
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

#ifndef KIS_OPENGL_CANVAS_2_H
#define KIS_OPENGL_CANVAS_2_H

#include <opengl/kis_opengl.h>

#ifdef HAVE_OPENGL

#include <QGLWidget>

#include <KoCanvasBase.h>

#include "canvas/kis_canvas_widget_base.h"
#include "opengl/kis_opengl_image_textures.h"

#include "krita_export.h"

class QWidget;
class QPaintEvent;
class KisCanvas2;
class KisDisplayFilter;

/**
 * KisOpenGLCanvas is the widget that shows the actual image using OpenGL
 *
 * NOTE: if you change something in the event handling here, also change it
 * in the qpainter canvas.
 *
 */
class KRITAUI_EXPORT KisOpenGLCanvas2 : public QGLWidget, public KisCanvasWidgetBase
{

    Q_OBJECT

public:

    KisOpenGLCanvas2(KisCanvas2 * canvas, KisCoordinatesConverter *coordinatesConverter, QWidget * parent, KisOpenGLImageTexturesSP imageTextures);

    virtual ~KisOpenGLCanvas2();

    void setDisplayFilter(KisDisplayFilter *displayFilter);

public: // QWidget

    /// reimplemented method from superclass
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

    /// reimplemented method from superclass
    virtual void inputMethodEvent(QInputMethodEvent *event);

    void renderCanvasGL() const;
    void renderDecorations();

private slots:
    void slotConfigChanged();

protected:

    void resizeGL(int width, int height);
    void initializeGL();
    void paintGL();

public: // KisAbstractCanvasWidget

    QWidget *widget() {
        return this;
    }

protected: // KisCanvasWidgetBase
    virtual bool callFocusNextPrevChild(bool next);

private:
    struct Private;
    Private * const d;

    void drawImage() const;
    void drawCheckers() const;

    void initializeCheckerShader();
    void initializeDisplayShader();
};

#endif // HAVE_OPENGL
#endif // KIS_OPENGL_CANVAS_2_H
