/*
 *  kis_glsl_widget.cc - part of KimageShop^WKrayon^WKrita
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

#include "kis_glsl_widget.h"

#include <QGLFramebufferObject>
#include <QMessageBox>

#include <klocale.h>
#include <kdebug.h>


KisGlslWidget::KisGlslWidget(KisPaintDeviceSP device, QWidget *parent) : QGLWidget(parent), m_device(device)
{
}

KisGlslWidget::~KisGlslWidget()
{
}

void KisGlslWidget::initializeGL()
{
    quint8* imgbuf;
    
    int err = glewInit();
  
    
    //if glew can't initialize, everything following is useless
    if(GLEW_OK != err) {
        qDebug("Unable to initialize glew, useful information follows");
        qDebug("OpenGL version: %s", glGetString(GL_VERSION));
        qDebug("Error: %s", glewGetErrorString(err));
        m_valid = false;
        return;
    }
    
    //if glew can't find support for the needed features, 
    //everything following is useless as well
    if (glewIsSupported("GL_VERSION_2_0") != GL_TRUE ||
        glewGetExtension("GL_ARB_fragment_shader")      != GL_TRUE ||
        glewGetExtension("GL_ARB_vertex_shader")        != GL_TRUE ||
        glewGetExtension("GL_ARB_shader_objects")       != GL_TRUE ||
        glewGetExtension("GL_ARB_shading_language_100") != GL_TRUE ||
        glewGetExtension("GL_EXT_framebuffer_object")   != GL_TRUE ||
        glewGetExtension("GL_ARB_texture_rectangle")    != GL_TRUE )
    {
       QMessageBox::warning( this, i18n( "Krita" ), i18n( "The OpenGL filter cannot run. Your graphics card or driver is missing the necessary extensions" ) );
        m_valid = false;
        return;
    }
    
    //get the bounds of the image
    m_bounds = m_device->exactBounds();
    
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glShadeModel(GL_SMOOTH); // Enables Smooth Shading
    glEnable(GL_DEPTH_TEST); // Enables Depth Testing
    glDepthFunc(GL_LEQUAL); // The Type Of Depth Test To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective
    
    //Setup orthogonal rendering
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0.0f, 0.0f, (GLfloat)m_bounds.width(),
               (GLfloat)m_bounds.height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0f, (GLfloat)m_bounds.width(), 0.0f, (GLfloat)m_bounds.height());
    
    //bind the texture from krita using readBytes
    m_device->readBytes(imgbuf, m_bounds);
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_texture);
    
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, m_bounds.width(), 
                 m_bounds.height(),0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8,
                 imgbuf);
    
}

void KisGlslWidget::resizeGL(int width, int height)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0f, (GLfloat)width, 0.0f, (GLfloat)height);
}

void KisGlslWidget::paintGL() 
{
    glActiveTexture(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, m_texture);
    
    glUseProgram(m_program);
    glUniform1i(glGetUniformLocation(m_program, "image"), 0);
    
    glBegin(GL_QUADS);
    
    glTexCoord2f(0.0f, 0.0f); 
    glVertex2f(0.0f, 0.0f);
    
    glTexCoord2f(0.0f, (GLfloat)m_bounds.height()); 
    glVertex2f(0.0f, (GLfloat)height());
    
    glTexCoord2f((GLfloat)m_bounds.width(), (GLfloat)m_bounds.height());
    glVertex2f((GLfloat)width(), (GLfloat)height());
               
    glTexCoord2f((GLfloat)m_bounds.width(), 0.0f);
    glVertex2f((GLfloat)width(), 0.0f);
    
    glEnd();
    
    glUseProgram(0);
}



#include "kis_glsl_widget.moc"

