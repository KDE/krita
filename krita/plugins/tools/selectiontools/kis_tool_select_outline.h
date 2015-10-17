/*
 *  kis_tool_select_freehand.h - part of Krayon^WKrita
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2015 Michael Abrahams <miabraha@gmail.com>
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

#ifndef KIS_TOOL_SELECT_OUTLINE_H_
#define KIS_TOOL_SELECT_OUTLINE_H_

#include <QPoint>
#include <KoToolFactoryBase.h>
#include "krita/ui/tool/kis_tool_select_base.h"
#include <kis_icon.h>

class QPainterPath;

class KisToolSelectOutline : public KisToolSelectBase
{


    Q_OBJECT
    Q_PROPERTY(int selectionAction READ selectionAction WRITE setSelectionAction NOTIFY selectionActionChanged)
    Q_SIGNALS: void selectionActionChanged();

public:
    KisToolSelectOutline(KoCanvasBase *canvas);
    virtual ~KisToolSelectOutline();
    void beginPrimaryAction(KoPointerEvent *event);
    void continuePrimaryAction(KoPointerEvent *event);
    void endPrimaryAction(KoPointerEvent *event);
    virtual void paint(QPainter& gc, const KoViewConverter &converter);


public Q_SLOTS:
    virtual void deactivate();
    void setSelectionAction(int newSelectionAction);

protected:
    using KisToolSelectBase::m_widgetHelper;

private:
    void updateFeedback();
    void updateCanvas();

    QPainterPath * m_paintPath;
    vQPointF m_points;
};

class KisToolSelectOutlineFactory : public KoToolFactoryBase
{
public:
    KisToolSelectOutlineFactory(const QStringList&)
            : KoToolFactoryBase("KisToolSelectOutline") {
        setToolTip(i18n("Outline Selection Tool"));
        setToolType(TOOL_TYPE_SELECTED);
        setIconName(koIconNameCStr("tool_outline_selection"));
        setPriority(55);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    virtual ~KisToolSelectOutlineFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolSelectOutline(canvas);
    }
};


#endif //__selecttoolfreehand_h__

