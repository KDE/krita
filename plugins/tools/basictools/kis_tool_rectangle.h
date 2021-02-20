/*
 *  kis_tool_rectangle.h - part of KImageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 1999 Michael Koch <koch@kde.org>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TOOL_RECTANGLE_H__
#define __KIS_TOOL_RECTANGLE_H__

#include "kis_tool_shape.h"
#include "kis_types.h"
#include "KisToolPaintFactoryBase.h"
#include "flake/kis_node_shape.h"
#include <kis_tool_rectangle_base.h>
#include <kis_icon.h>


class QRect;

class KoCanvasBase;

class KisToolRectangle : public KisToolRectangleBase
{
    Q_OBJECT

public:
    KisToolRectangle(KoCanvasBase * canvas);
    ~KisToolRectangle() override;

protected:
    void finishRect(const QRectF& rect, qreal roundCornersX, qreal roundCornersY) override;

protected Q_SLOTS:
    void resetCursorStyle() override;
};

class KisToolRectangleFactory : public KisToolPaintFactoryBase
{

public:
    KisToolRectangleFactory()
            : KisToolPaintFactoryBase("KritaShape/KisToolRectangle") {
        setToolTip(i18n("Rectangle Tool"));

        setSection(TOOL_TYPE_SHAPE);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("krita_tool_rectangle"));
        //setShortcut( Qt::Key_F6 );
        setPriority(2);
    }

    ~KisToolRectangleFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return  new KisToolRectangle(canvas);
    }

};


#endif // __KIS_TOOL_RECTANGLE_H__

