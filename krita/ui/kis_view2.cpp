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

#include <KoCanvasController.h>

#include "kis_view_converter.h"
#include "kis_canvas2.h"
#include "kis_opengl_canvas2.h"
#include "kis_qpainter_canvas.h"
#include "kis_doc2.h"
#include "kis_view2.h"


KisView2::KisView2(KisDoc2 * doc,  QWidget * parent)
    : KoView(doc, parent)
    , m_QPainterCanvas( 0 )
    , m_openGLCanvas( 0 )
    , m_doc( doc )
    , m_canvasController( new KoCanvasController( this ) )

{

    m_viewConverter = new KisViewConverter(1.0, 100, 96, 96);
    m_openGLCanvas = new KisOpenGLCanvas2(this);
    m_QPainterCanvas = new KisQPainterCanvas(this);
    m_canvas = new KisCanvas2( m_viewConverter, m_QPainterCanvas );
    m_canvasController->setCanvas( m_canvas );
}


KisView2::~KisView2()
{
}

#include "kis_view2.moc"
