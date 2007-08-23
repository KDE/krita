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
#include <kis_layer.h>
#include "kis_glsl_widget.h"


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
    KisGlslWidget * glslview;
    QScrollArea * scrollArea;
    KisPaintDeviceSP dev;
    int maxTextureSize;
    GLuint fb;
};


DlgGlsl::DlgGlsl( KisView2 *  parent,
                  const char * name)
    : KDialog (parent)
    , m_d( new Private )
{
    setCaption( i18n("Glsl Image") );
    setButtons(  Ok | Cancel);
    setDefaultButton( Ok );
    setObjectName(name);

    m_d->view = parent;
    m_d->page = new WdgGlsl(this);
    m_d->page->setObjectName("glsl");

    QVBoxLayout *vbox = new QVBoxLayout;

    m_d->scrollArea = new QScrollArea;
    m_d->scrollArea->setBackgroundRole(QPalette::Dark);

    vbox->addWidget( m_d->scrollArea );
    m_d->page->grpPreview->setLayout( vbox );

    m_d->glslview = new KisGlslWidget(m_d->view->activeLayer()->paintDevice(), m_d->scrollArea);
    m_d->glslview->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_d->scrollArea->setWidget(m_d->glslview);
    
    if(!m_d->glslview->isValidGLSL()) {
        close();
    }

    setMainWidget(m_d->page);
    resize(m_d->page->sizeHint());

    connect(this, SIGNAL(okClicked()),
            this, SLOT(okClicked()));

    connect( m_d->page->bnPreview,  SIGNAL( clicked() ),
             this,  SLOT( resetPreview() ) );


}

DlgGlsl::~DlgGlsl()
{
    delete m_d;
}

void DlgGlsl::okClicked()
{
    accept();
}

void DlgGlsl::resetPreview()
{
    m_d->glslview->slotShaders(m_d->page->fragmentText->toPlainText(), m_d->page->vertexText->toPlainText());
    m_d->glslview->updateGL();
}

#include "dlg_glsl.moc"
