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
#include "KoToolFactoryBase.h"
#include "flake/kis_node_shape.h"
#include <kis_tool_ellipse_base.h>
#include <kis_icon.h>



class KoCanvasBase;

class KisToolEllipse : public KisToolEllipseBase
{
    Q_OBJECT

public:
    KisToolEllipse(KoCanvasBase * canvas);
    ~KisToolEllipse() override;

protected Q_SLOTS:
    void resetCursorStyle() override;

protected:
    void finishRect(const QRectF& rect, qreal roundCornersX, qreal roundCornersY) override;
};

class KisToolEllipseFactory : public KoToolFactoryBase
{

public:
    KisToolEllipseFactory()
            : KoToolFactoryBase("KritaShape/KisToolEllipse") {
        setToolTip(i18n("Ellipse Tool"));
        setSection(TOOL_TYPE_SHAPE);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("krita_tool_ellipse"));
        setPriority(3);
    }

    ~KisToolEllipseFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return  new KisToolEllipse(canvas);
    }

};


#endif //__KIS_TOOL_ELLIPSE_H__

