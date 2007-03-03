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
#include "config-krita.h"
#ifdef HAVE_OPENGL

#include <QGLWidget>

#include <KoCanvasBase.h>

#include "kis_abstract_canvas_widget.h"
#include "kis_opengl_image_context.h"

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

    KisOpenGLCanvas2( KisCanvas2 * canvas, QWidget * parent, KisOpenGLImageContextSP context);

    virtual ~KisOpenGLCanvas2();

protected:

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

public: // KisAbstractCanvasWidget

    QWidget * widget() { return this; }

    KoToolProxy * toolProxy();

    void documentOffsetMoved( QPoint pt );

private:
    class Private;
    Private * m_d;

};

#endif // HAVE_OPENGL
#endif // KIS_OPENGL_CANVAS_2_H
