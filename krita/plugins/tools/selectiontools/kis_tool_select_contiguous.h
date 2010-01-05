/*
 *  kis_tool_select_contiguous.h - part of KImageShop^WKrayon^Krita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#ifndef __KIS_TOOL_SELECT_CONTIGUOUS_H__
#define __KIS_TOOL_SELECT_CONTIGUOUS_H__

#include "KoToolFactory.h"
#include "krita/ui/tool/kis_tool_select_base.h"


/**
 * The 'magic wand' selection tool -- in fact just
 * a floodfill that only creates a selection.
 */
class KisToolSelectContiguous : public KisToolSelectBase
{

    Q_OBJECT

public:
    KisToolSelectContiguous(KoCanvasBase *canvas);
    virtual ~KisToolSelectContiguous();

    virtual QWidget* createOptionWidget();
    virtual void paint(QPainter &painter, const KoViewConverter &converter);

    virtual void mousePressEvent(KoPointerEvent *event);

public slots:
    virtual void slotSetFuzziness(int);
    virtual void slotLimitToCurrentLayer(int);

private:
    int m_fuzziness;
    bool m_limitToCurrentLayer;
};

class KisToolSelectContiguousFactory : public KoToolFactory
{

public:
    KisToolSelectContiguousFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KisToolSelectContiguous") {
        setToolTip(i18n("Select a contiguous area of colors"));
        setToolType(TOOL_TYPE_SELECTED);
        setIcon("tool_contiguous_selection");
        setPriority(56);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
    }

    virtual ~KisToolSelectContiguousFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolSelectContiguous(canvas);
    }

};

#endif //__KIS_TOOL_SELECT_CONTIGUOUS_H__

