/* This file is part of the KDE project
   Copyright 2007 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef PICTURE_TOOL
#define PICTURE_TOOL

#include <KoTool.h>

class KritaShapeTool : public KoTool
{
    Q_OBJECT
public:
    explicit KritaShapeTool( KoCanvasBase* canvas );
    ~KritaShapeTool();

    virtual void paint( QPainter& painter, KoViewConverter& converter );

    virtual void mousePressEvent( KoPointerEvent* event ) ;
    virtual void mouseMoveEvent( KoPointerEvent* event );
    virtual void mouseReleaseEvent( KoPointerEvent* event );

    void activate (bool temporary=false);
    void deactivate();

protected:
    /*
     * Create default option widget
     */
    virtual QWidget * createOptionWidget();

protected slots:

   void slotChangeUrl();

private:
    KritaShape *m_kritaShapeshape;
};

#endif
