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
#include "kis_delegated_tool.h"
#include <kis_icon.h>

class KoCanvasBase;
class KisToolPencil;

///

class __KisToolPencilLocalTool : public KoPencilTool {
public:
    __KisToolPencilLocalTool(KoCanvasBase * canvas, KisToolPencil* parentTool);
    void paint(QPainter &painter, const KoViewConverter &converter) override;
    virtual void paintPath(KoPathShape * path, QPainter &painter, const KoViewConverter &converter);
    void addPathShape(KoPathShape* pathShape, bool closePath) override;

    using KoPencilTool::createOptionWidgets;

protected:
    void slotUpdatePencilCursor() override;

private:
    KisToolPencil* const m_parentTool;
};

typedef KisDelegatedTool<KisToolShape,
__KisToolPencilLocalTool,
DeselectShapesActivationPolicy> DelegatedPencilTool;

class KisToolPencil : public DelegatedPencilTool
{
    Q_OBJECT

public:
    KisToolPencil(KoCanvasBase * canvas);
    void mousePressEvent(KoPointerEvent *event) override;

    QList<QPointer<QWidget> > createOptionWidgets() override;

protected Q_SLOTS:
    void resetCursorStyle() override;

private:
    void updatePencilCursor(bool value);

private:
    friend class __KisToolPencilLocalTool;
};

class KisToolPencilFactory : public KoToolFactoryBase
{

public:
    KisToolPencilFactory()
        : KoToolFactoryBase("KisToolPencil") {
        setToolTip(i18n("Freehand Path Tool"));
        setSection(TOOL_TYPE_SHAPE);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("krita_tool_freehandvector"));
        setPriority(9);
    }

    ~KisToolPencilFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolPencil(canvas);
    }
};



#endif // KIS_TOOL_PENCIL_H_

