/*
 *  dlg_glsl.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include <gl/glew.h>
#include <QGLWidget>

class KisOpenGLShader;


class KisGlslWidget : public QGLWidget
{
    public:
        KisGlslWidget(QWidget *parent = 0);
        ~KisGlslWidget();
        
        void paintGL();
        void resizeGL(int width, int height);
        void initializeGL();
        
        GLuint bindTexture(const quint8* buffer, const quint32 width, 
                           const quint32 height, GLenum target = GL_TEXTURE_2D, 
                           GLint format = GL_RGBA);
        
        bool isValidGLSL() { return m_valid; };
        
    public slots:
        void fragmentShaderSlot(const QString& shader);
        void vertexShaderSlot(const QString& shader);
     
    private:
        GLuint fragshader, vertexshader, fragprogram, vertexprogram;
        QGLFramebufferObject framebuffer;
        bool m_valid;
}       

#endif
