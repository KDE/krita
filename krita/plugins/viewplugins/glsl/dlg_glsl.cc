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
#include "dlg_glsl.h"

#include <GL/glew.h>
#include <GL/glut.h>
#include <QtOpenGL>


#include <QScrollArea>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>

#include <klocale.h>
#include <kdebug.h>

#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_view2.h>


// This shader performs a 9-tap Laplacian edge detection filter.
// (converted from the separate "edges.cg" file to embedded GLSL string)
static const char *edgeFragSource = {
"uniform sampler2D texUnit;"
"void main(void)"
"{"
"   const float offset = 1.0 / 512.0;"
"   vec2 texCoord = gl_TexCoord[0].xy;"
"   vec4 c  = texture2D(texUnit, texCoord);"
"   vec4 bl = texture2D(texUnit, texCoord + vec2(-offset, -offset));"
"   vec4 l  = texture2D(texUnit, texCoord + vec2(-offset,     0.0));"
"   vec4 tl = texture2D(texUnit, texCoord + vec2(-offset,  offset));"
"   vec4 t  = texture2D(texUnit, texCoord + vec2(    0.0,  offset));"
"   vec4 ur = texture2D(texUnit, texCoord + vec2( offset,  offset));"
"   vec4 r  = texture2D(texUnit, texCoord + vec2( offset,     0.0));"
"   vec4 br = texture2D(texUnit, texCoord + vec2( offset,  offset));"
"   vec4 b  = texture2D(texUnit, texCoord + vec2(    0.0, -offset));"
"   gl_FragColor = 8.0 * (c + -0.125 * (bl + l + tl + t + ur + r + br + b));"
"}"
};

class DlgGlsl::Private
{
public:
    KisView2 * view;
    WdgGlsl * page;
    QLabel * imageLabel;
    QScrollArea * scrollArea;
    KisPaintDeviceSP dev;
    int maxTextureSize;
    GLuint fb;
};


DlgGlsl::DlgGlsl( KisView2 *  parent,
                  const char * name)
    : KDialog (parent)
{
    setCaption( i18n("Glsl Image") );
    setButtons(  Ok | Cancel);
    setDefaultButton( Ok );
    setObjectName(name);

    m_d = new Private();

    m_d->view = parent;
    m_d->page = new WdgGlsl(this);
    m_d->page->setObjectName("glsl");

    QVBoxLayout *vbox = new QVBoxLayout;

    m_d->scrollArea = new QScrollArea;
    m_d->scrollArea->setBackgroundRole(QPalette::Dark);

    vbox->addWidget( m_d->scrollArea );
    m_d->page->grpPreview->setLayout( vbox );

    m_d->imageLabel = new QLabel;
    m_d->imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_d->imageLabel->setScaledContents(true);

    if ( m_d->dev = m_d->view->image()->activeDevice() )
        m_d->imageLabel->setPixmap( QPixmap::fromImage( m_d->dev->convertToQImage( 0 ) ) );

    m_d->scrollArea->setWidget(m_d->imageLabel);

    setMainWidget(m_d->page);
    resize(m_d->page->sizeHint());

    connect(this, SIGNAL(okClicked()),
            this, SLOT(okClicked()));

    connect( m_d->page->bnPreview,  SIGNAL( clicked() ),
             this,  SLOT( resetPreview() ) );

    if ( !setupGL() ) close();

}

DlgGlsl::~DlgGlsl()
{
    delete m_d;
}


bool DlgGlsl::setupGL()
{
    int argc = 0;

    glutInit ( &argc, 0 );
    glutCreateWindow("test");

    glewInit();

    if (glewIsSupported("GL_VERSION_2_0") != GL_TRUE ||
        glewGetExtension("GL_ARB_fragment_shader")      != GL_TRUE ||
        glewGetExtension("GL_ARB_vertex_shader")        != GL_TRUE ||
        glewGetExtension("GL_ARB_shader_objects")       != GL_TRUE ||
        glewGetExtension("GL_ARB_shading_language_100") != GL_TRUE ||
        glewGetExtension("GL_EXT_framebuffer_object")   != GL_TRUE ||
        glewGetExtension("GL_ARB_texture_rectangle")    != GL_TRUE )
    {
        QMessageBox::warning( this, i18n( "Krita" ), i18n( "The OpenGL filter cannot run. Your graphics card or driver is missing the necessary extensions" ) );
        return false;
    }

    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH); // Enables Smooth Shading
    glEnable(GL_DEPTH_TEST); // Enables Depth Testing
    glDepthFunc(GL_LEQUAL); // The Type Of Depth Test To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Really Nice Perspective Calculations

    // create FBO (off-screen framebuffer)
    glGenFramebuffersEXT(1, &m_d->fb);
    // bind offscreen buffer
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_d->fb);

    glGetIntegerv(GL_MAX_TEXTURE_SIZE,&m_d->maxTextureSize);
    kDebug() << "GL_MAX_TEXTURE_SIZE: " << m_d->maxTextureSize << endl;

    return true;
}

void DlgGlsl::okClicked()
{
    accept();
}

void DlgGlsl::resetPreview()
{
    // Create texture from image (for now, just all of it, provided
    // it's smaller than maxtexturesize)
}

#include "dlg_glsl.moc"
