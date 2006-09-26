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

class KisCanvas;
class KisImage;
class KisViewConverter;
class KisZoomHandler;
class KoCanvasController;
clsas KisDoc;

class KisView : public KoView {

Q_OBJECT

public:

    KisView(KisDoc * doc, QWidget * parent);
    virtual ~KisView();

public:

    // KoView implementation
    virtual void updateReadWrite( bool readwrite ) {};

public:
    // Krita specific interfaces

private:

    KisCanvas * m_openGLCanvas;
    KisCanvas * m_QPainterCanvas;
    KisDoc * m_doc;
    KisZoomHandler * m_zoomHandler;
    KisViewConverter * m_viewConverter;
    KoCanvasController * m_canvasController;

};

#endif
