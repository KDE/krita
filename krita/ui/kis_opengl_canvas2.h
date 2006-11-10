/*
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_OPENGL_CANVAS_2_H
#define KIS_OPENGL_CANVAS_2_H

#ifdef HAVE_OPENGL

#include <QGLWidget>

#include <KoCanvasBase.h>

#include "kis_abstract_canvas_widget.h"

class QWidget;
class QGLContext;
class QPaintEvent;
class QImage;
class QBrush;
class KisCanvas2;

/**
 * KisOpenGLCanvas is the widget that shows the actual image using OpenGL
 *
 */
class KisOpenGLCanvas2 : public QGLWidget, public KisAbstractCanvasWidget
{

    Q_OBJECT

public:

    KisOpenGLCanvas2( KisCanvas2 * canvas, QWidget * parent );

    KisOpenGLCanvas2(KisCanvas2 * canvas, QGLContext * context, QWidget * parent, QGLWidget *sharedContextWidget);

    virtual ~KisOpenGLCanvas2();

protected:
    void initializeGL();
    void resizeGL(int w, int h);

public: // QWidget

    void paintEvent ( QPaintEvent * event );

public: // KisAbstractCanvasWidget

    QWidget * widget() { return this; }

    KoToolProxy * toolProxy() {
        return m_toolProxy;
    }
private:
    KisCanvas2 * m_canvas;
    QImage * m_checkTexture;
    QBrush * m_checkBrush;
    KoToolProxy * m_toolProxy;

};

#endif
#endif
