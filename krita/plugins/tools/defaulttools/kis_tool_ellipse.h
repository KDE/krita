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
#include "kis_types.h"
#include "KoToolFactory.h"
#include "flake/kis_node_shape.h"
#include <kis_tool_ellipse_base.h>


class QPainter;

class KoCanvasBase;

class KisToolEllipse : public KisToolEllipseBase
{
    Q_OBJECT

public:
    KisToolEllipse(KoCanvasBase * canvas);
    virtual ~KisToolEllipse();

protected:
    virtual void finishEllipse(const QRectF& rect);
};

class KisToolEllipseFactory : public KoToolFactory
{

public:
    KisToolEllipseFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KritaShape/KisToolEllipse") {
        setToolTip(i18n("Draw an ellipse"));
        setToolType(TOOL_TYPE_SHAPE);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
        setIcon("krita_tool_ellipse");
        setPriority(3);
        setInputDeviceAgnostic(false);
    }

    virtual ~KisToolEllipseFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return  new KisToolEllipse(canvas);
    }

};


#endif //__KIS_TOOL_ELLIPSE_H__

