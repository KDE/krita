/* This file is part of the KDE project
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


#include "kis_opengl_canvas2.h"

#include <QtOpenGL>
#include <QWidget>
#include <QGLWidget>
#include <QGLContext>
#include <QImage>
#include <QBrush>
#include <QPainter>
#include <QPaintEvent>

#include <kdebug.h>

#define PATTERN_WIDTH 64
#define PATTERN_HEIGHT 64

KisOpenGLCanvas2::KisOpenGLCanvas2( KisCanvas2 * canvas, QWidget * parent )
    : QGLWidget( QGLFormat(QGL::SampleBuffers), parent )
    , m_canvas( canvas )
{

    setAttribute(Qt::WA_NoSystemBackground);

    m_checkTexture = new QImage(PATTERN_WIDTH, PATTERN_HEIGHT, QImage::Format_RGB32);

    for (int y = 0; y < PATTERN_HEIGHT; y++)
    {
        for (int x = 0; x < PATTERN_WIDTH; x++)
        {
            quint8 v = 128 + 63 * ((x / 16 + y / 16) % 2);
            m_checkTexture->setPixel(x, y, qRgb(v, v, v));
        }
    }

    m_checkBrush = new QBrush( *m_checkTexture );
}

KisOpenGLCanvas2::KisOpenGLCanvas2(KisCanvas2 * canvas, QGLContext * context, QWidget * parent, QGLWidget *sharedContextWidget)
    : QGLWidget( context, parent, sharedContextWidget )
    , m_canvas( canvas )
{
}


KisOpenGLCanvas2::~KisOpenGLCanvas2()
{
    delete m_checkTexture;
    delete m_checkBrush;
}

void KisOpenGLCanvas2::initializeGL()
{
    qglClearColor(QColor::fromCmykF(0.40, 0.0, 1.0, 0.0));
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void KisOpenGLCanvas2::resizeGL(int w, int h)
{
    kDebug() << "Resize gl to " << w << ", " << h << endl;
    glViewport(0, 0, (GLint)w, (GLint)h);
}

void KisOpenGLCanvas2::paintEvent( QPaintEvent * ev )
{
    QPainter gc;
    gc.setRenderHint(QPainter::Antialiasing);
    gc.begin( this );
    gc.fillRect( ev->rect(), *m_checkBrush );
    gc.end();
}


#include "kis_opengl_canvas2.moc"
