/*
 *  SPDX-FileCopyrightText: 2012 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
        setSection(ToolBoxSection::Shape);
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

