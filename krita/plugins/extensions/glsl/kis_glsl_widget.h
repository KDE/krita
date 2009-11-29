/*
 *  kis_glsl_widget.h - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2007 Thomas Burdick <tburdi1@uic.edu>
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

#ifndef _KIS_GLSL_WIDGET
#define _KIS_GLSL_WIDGET

#include <GL/glew.h>
#include <QGLWidget>
#include <QGLFramebufferObject>

#include <kis_paint_device.h>

class QGLFramebufferObject;
class QString;


class KisGlslWidget : public QGLWidget
{
    Q_OBJECT

public:
    KisGlslWidget(KisPaintDeviceSP device, QWidget *parent = 0);
    ~KisGlslWidget();

    void paintGL();
    void resizeGL(int width, int height);
    void initializeGL();

    bool isValidGLSL() const {
        return m_valid;
    }

public slots:
    void slotShaders(const QString &fragmentShader, const QString &vertexShader);

private:

    GLuint m_texture, m_fragshader, m_vertexshader, m_program;
    QGLFramebufferObject *m_framebuffer;
    bool m_valid;
    KisPaintDeviceSP m_device;
    QRect m_bounds;
    quint8* m_imagebuf;
};

#endif
