/*
 *  kis_tool_select_elliptical.h - part of Krayon^WKrita
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org> *
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

#ifndef __KIS_TOOL_SELECT_ELLIPTICAL_H__
#define __KIS_TOOL_SELECT_ELLIPTICAL_H__

#include "KoToolFactoryBase.h"
#include "kis_tool_ellipse_base.h"
#include <kis_tool_select_base.h>
#include "kis_selection_tool_config_widget_helper.h"
#include <KoIcon.h>
#include <QKeySequence>
#include <kis_icon.h>
#include <QMenu>



class __KisToolSelectEllipticalLocal : public KisToolEllipseBase
{
    Q_OBJECT

public:
    __KisToolSelectEllipticalLocal(KoCanvasBase *canvas);
protected:
    virtual SelectionMode selectionMode() const = 0;
    virtual SelectionAction selectionAction() const = 0;
    virtual bool antiAliasSelection() const = 0;
private:
    void finishRect(const QRectF &rect, qreal roundCornersX, qreal roundCornersY) override;



};



typedef KisToolSelectBase<__KisToolSelectEllipticalLocal> KisToolSelectEllipticalTemplate;

class KisToolSelectElliptical : public KisToolSelectEllipticalTemplate
{
    Q_OBJECT
public:
    KisToolSelectElliptical(KoCanvasBase* canvas);
    QMenu* popupActionsMenu() override;

public Q_SLOTS:
    void setSelectionAction(int);
};

class KisToolSelectEllipticalFactory : public KoToolFactoryBase
{
public:
    KisToolSelectEllipticalFactory()
        : KoToolFactoryBase("KisToolSelectElliptical")
    {
        setToolTip(i18n("Elliptical Selection Tool"));
        setSection(TOOL_TYPE_SELECTION);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("tool_elliptical_selection"));
        setShortcut(QKeySequence(Qt::Key_J));
        setPriority(1);
    }

    ~KisToolSelectEllipticalFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolSelectElliptical(canvas);
    }

};

#endif //__KIS_TOOL_SELECT_ELLIPTICAL_H__

