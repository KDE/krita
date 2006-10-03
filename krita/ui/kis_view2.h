/*
 *  Copyright (c) 2006 Boudewijn Rempt
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

#ifndef KIS_VIEW_2
#define KIS_VIEW_2

#include <KoView.h>

class KisCanvas2;
class KisQPainterCanvas;
class KisOpenGLCanvas2;
class KisImage;
class KisViewConverter;
class KoCanvasController;
class KisDoc2;

class KisView2 : public KoView {

Q_OBJECT

public:

    KisView2(KisDoc2 * doc, QWidget * parent);
    virtual ~KisView2();

public:

    // KoView implementation
    virtual void updateReadWrite( bool readwrite ) { Q_UNUSED(readwrite); }

public:
    // Krita specific interfaces

private:

    KisCanvas2 * m_canvas;
    KisQPainterCanvas * m_QPainterCanvas;
    KisOpenGLCanvas2 * m_openGLCanvas;
    KisDoc2 * m_doc;
    KisViewConverter * m_viewConverter;
    KoCanvasController * m_canvasController;

};

#endif
