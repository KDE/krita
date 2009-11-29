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

#include <KoColorSpace.h>

#include <klocale.h>
#include <kis_debug.h>


KisGlslWidget::KisGlslWidget(KisPaintDeviceSP device, QWidget *parent) : QGLWidget(parent), m_device(device)
{
    int bufsize = 0;
    //get the bounds of the image
    m_bounds = m_device->exactBounds();

    bufsize = m_device->colorSpace()->pixelSize() * m_bounds.width() * m_bounds.height();
    m_imagebuf = new quint8[bufsize];
}

KisGlslWidget::~KisGlslWidget()
{
    delete[] m_imagebuf;
}

void KisGlslWidget::initializeGL()
{
    m_valid = true;

    int err = glewInit();


    //if glew can't initialize, everything following is useless
    if (GLEW_OK != err) {

        qDebug("Unable to initialize glew, useful information follows");
        qDebug("OpenGL version: %s", glGetString(GL_VERSION));
        qDebug("Error: %s", glewGetErrorString(err));
        QMessageBox::warning(this, i18n("Krita"), i18n("Cannot run GLSL programs on this computer"));
        QMessageBox::warning(this, i18n("Krita"), (char*)glewGetErrorString(err));
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
            glewGetExtension("GL_ARB_texture_rectangle")    != GL_TRUE) {
        QMessageBox::warning(this, i18n("Krita"), i18n("The OpenGL filter cannot run. Your graphics card or driver is missing the necessary extensions"));
        m_valid = false;
        return;
    }



    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glShadeModel(GL_SMOOTH); // Enables Smooth Shading
    glEnable(GL_DEPTH_TEST); // Enables Depth Testing
    glDepthFunc(GL_LEQUAL); // The Type Of Depth Test To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective

    glClearColor(0.5, 0.5, 0.5, 0.0);

    //Setup orthogonal rendering
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0.0f, 0.0f, (GLfloat)m_bounds.width(),
               (GLfloat)m_bounds.height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0f, (GLfloat)m_bounds.width(), 0.0f, (GLfloat)m_bounds.height());

    //bind the texture from krita using readBytes
    m_device->readBytes(m_imagebuf, m_bounds);
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
                 m_bounds.height(), 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8,
                 m_imagebuf);

    m_fragshader = glCreateShader(GL_FRAGMENT_SHADER);
    m_vertexshader = glCreateShader(GL_VERTEX_SHADER);
    m_program = glCreateProgram();

    //important, make the size of the widget correct
    resize(m_bounds.width(), m_bounds.height());

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
    glClear(GL_COLOR_BUFFER_BIT);

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

void KisGlslWidget::slotShaders(const QString &fragShader, const QString &vertShader)
{
    bool vertvalid = true;
    bool fragvalid = true;
    const char * sourcetmp[1];
    GLint compiled = 1;
    GLsizei loglength;
    const int logbufsize = 1024;
    GLchar *log = new GLchar[logbufsize];


    QMessageBox::warning(this, i18n("Krita"), i18n("Setting up Shader"));
    QByteArray b = fragShader.toAscii();
    sourcetmp[0] = b.constData();

    glShaderSource(m_fragshader, 1, sourcetmp, NULL);
    glCompileShader(m_fragshader);

    glGetShaderiv(m_fragshader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        glGetShaderInfoLog(m_fragshader, logbufsize, &loglength, log);
        QMessageBox::warning(this, i18n("Krita"), i18n("There is an error in your Fragment Shader"));
        QMessageBox::warning(this, i18n("Krita"), (char *)log);
        fragvalid = false;

    }
    compiled = 1;

    b = vertShader.toAscii();
    sourcetmp[0] = b.constData();

    glShaderSource(m_vertexshader, 1, sourcetmp, NULL);
    glCompileShader(m_vertexshader);

    glGetShaderiv(m_vertexshader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        glGetShaderInfoLog(m_vertexshader, logbufsize, &loglength, log);
        QMessageBox::warning(this, i18n("Krita"), i18n("There is an error in your Vertex Shader"));
        qDebug("Vertex shader log: %s", log);
        vertvalid = false;

    }
    compiled = 1;

    if (fragvalid) {
        glAttachShader(m_program, m_fragshader);
    }

    if (vertvalid) {
        glAttachShader(m_program, m_vertexshader);
    }

    glLinkProgram(m_program);

    glGetProgramiv(m_program, GL_LINK_STATUS, &compiled);

    if (!compiled) {
        glGetProgramInfoLog(m_program, logbufsize, &loglength, log);
        QMessageBox::warning(this, i18n("Krita"), i18n("There is an error with your GLSL Program, it cannot be linked"));
        qDebug("Program shader log: %s", log);
        m_valid = false;

    }
    QMessageBox::warning(this, i18n("Krita"), i18n("The shader should run!"));

    glDraw();

    delete log;

}

#include "kis_glsl_widget.moc"

