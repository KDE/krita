/*
 *  kis_tool_ellipse.h - part of Krayon
 *
 *  SPDX-FileCopyrightText: 2000 John Califf <jcaliff@compuzone.net>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TOOL_ELLIPSE_H__
#define __KIS_TOOL_ELLIPSE_H__

#include "kis_tool_shape.h"
#include "kis_types.h"
#include "KisToolPaintFactoryBase.h"
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

class KisToolEllipseFactory : public KisToolPaintFactoryBase
{

public:
    KisToolEllipseFactory()
            : KisToolPaintFactoryBase("KritaShape/KisToolEllipse") {
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

