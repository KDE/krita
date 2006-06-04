/*
 *  kis_tool_ellipse.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
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

#ifndef __KIS_TOOL_ELLIPSE_H__
#define __KIS_TOOL_ELLIPSE_H__

#include "kis_tool_shape.h"

class KisCanvas;
class KisPainter;
class KisRect;

class KisToolEllipse : public KisToolShape {

    typedef KisToolShape super;
    Q_OBJECT

public:
    KisToolEllipse();
    virtual ~KisToolEllipse();

        //
        // KisCanvasObserver interface
        //

        virtual void update (KisCanvasSubject *subject);

        //
        // KisToolPaint interface
        //

    virtual void setup(KActionCollection *collection);
    virtual quint32 priority() { return 3; }
    virtual enumToolType toolType() { return TOOL_SHAPE; }
    virtual void buttonPress(KisButtonPressEvent *event);
    virtual void move(KisMoveEvent *event);
    virtual void buttonRelease(KisButtonReleaseEvent *event);

protected:
    virtual void draw(const KisPoint& start, const KisPoint& stop);

protected:
    KisPoint m_dragCenter;
    KisPoint m_dragStart;
    KisPoint m_dragEnd;

    bool m_dragging;
    KisImageSP m_currentImage;
};


#include "kis_tool_factory.h"

class KisToolEllipseFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolEllipseFactory() : super() {};
    virtual ~KisToolEllipseFactory(){};
    
    virtual KisTool * createTool(KActionCollection * ac) { 
        KisTool * t =  new KisToolEllipse(); 
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t; 
    }
    virtual KoID id() { return KoID("ellipse", i18n("Ellipse Tool")); }
};


#endif //__KIS_TOOL_ELLIPSE_H__

