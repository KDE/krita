/*
 *  kis_tool_rectangle.h - part of KImageShop^WKrayon^WKrita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef __KIS_TOOL_RECTANGLE_H__
#define __KIS_TOOL_RECTANGLE_H__

#include <QRect>

#include "kis_tool_shape.h"
#include "kis_types.h"
#include "kis_tool_factory.h"
#include "kis_point.h"

class QPainter;
class KisPainter;

class KisToolRectangle : public KisToolShape {

    typedef KisToolShape super;
    Q_OBJECT

public:
    KisToolRectangle();
    virtual ~KisToolRectangle();

        //
        // KisCanvasObserver interface
        //

        virtual void update (KisCanvasSubject *subject);

        //
        // KisToolPaint interface
        //

    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_SHAPE; }
    virtual quint32 priority() { return 2; }
    virtual void buttonPress(KisButtonPressEvent *event);
    virtual void move(KisMoveEvent *event);
    virtual void buttonRelease(KisButtonReleaseEvent *event);

protected:
    virtual void draw(const KisPoint&, const KisPoint&);

protected:
    int m_lineThickness;

    KisPoint m_dragCenter;
    KisPoint m_dragStart;
    KisPoint m_dragEnd;
    QRect m_final_lines;

    bool m_dragging;
    KisImageSP m_currentImage;
};

class KisToolRectangleFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolRectangleFactory() : super() {};
    virtual ~KisToolRectangleFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolRectangle();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KisID id() { return KisID("rectangle", i18n("Rectangle Tool")); }
};


#endif // __KIS_TOOL_RECTANGLE_H__

