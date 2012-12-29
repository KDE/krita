/*
 *  Copyright (c) 2012 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef KIS_TOOL_PENCIL_H_
#define KIS_TOOL_PENCIL_H_

#include <KoPencilTool.h>
#include <KoToolFactoryBase.h>

#include "flake/kis_node_shape.h"
#include "kis_tool_shape.h"
#include <KoIcon.h>

class KisSelectionOptions;
class KoCanvasBase;

class KisToolPencil : public KisToolShape
{

    Q_OBJECT

public:
    KisToolPencil(KoCanvasBase * canvas);
    virtual ~KisToolPencil();

    virtual void paint(QPainter &painter, const KoViewConverter &converter);
    void mousePressEvent(KoPointerEvent *event);
    void mouseDoubleClickEvent(KoPointerEvent *event);
    void mouseMoveEvent(KoPointerEvent *event);
    void mouseReleaseEvent(KoPointerEvent *event);
    
public slots:
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    virtual void deactivate();

private:
    /// reimplemented
    virtual QList<QWidget *> createOptionWidgets();

    class LocalTool : public KoPencilTool {
        friend class KisToolPencil;
    public:
        LocalTool(KoCanvasBase * canvas, KisToolPencil* selectingTool);
        virtual void paintPath(KoPathShape &path, QPainter &painter, const KoViewConverter &converter);
        virtual void addPathShape(KoPathShape* pathShape, bool closePath);
    private:
        KisToolPencil* const m_parentTool;
    };
    LocalTool* const m_localTool;

};

class KisToolPencilFactory : public KoToolFactoryBase
{

public:
    KisToolPencilFactory(const QStringList&)
            : KoToolFactoryBase("KisToolPencil") {
        setToolTip(i18n("Draw a freehand path."));
        setToolType(TOOL_TYPE_SHAPE);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("krita_tool_freehand"));
        setPriority(9);
    }

    virtual ~KisToolPencilFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolPencil(canvas);
    }
};



#endif // KIS_TOOL_PENCIL_H_

