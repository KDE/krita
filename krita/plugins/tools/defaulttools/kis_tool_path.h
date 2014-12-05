/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef KIS_TOOL_PATH_H_
#define KIS_TOOL_PATH_H_

#include <KoCreatePathTool.h>
#include <KoToolFactoryBase.h>

#include "flake/kis_node_shape.h"
#include "kis_tool_shape.h"
#include "kis_delegated_tool.h"
#include <KoIcon.h>

class KoCanvasBase;
class KisToolPath;


class __KisToolPathLocalTool : public KoCreatePathTool {
public:
    __KisToolPathLocalTool(KoCanvasBase * canvas, KisToolPath* parentTool);

    virtual void paintPath(KoPathShape &path, QPainter &painter, const KoViewConverter &converter);
    virtual void addPathShape(KoPathShape* pathShape);

    using KoCreatePathTool::createOptionWidgets;
    using KoCreatePathTool::endPathWithoutLastPoint;
    using KoCreatePathTool::endPath;
    using KoCreatePathTool::cancelPath;

private:
    KisToolPath* const m_parentTool;
};

typedef KisDelegatedTool<KisToolShape,
                         __KisToolPathLocalTool,
                         DeselectShapesActivationPolicy> DelegatedPathTool;

class KisToolPath : public DelegatedPathTool
{
    Q_OBJECT

public:
    KisToolPath(KoCanvasBase * canvas);
    void mousePressEvent(KoPointerEvent *event);

    virtual QList< QPointer<QWidget> > createOptionWidgets();

protected:
    void requestStrokeCancellation();
    void requestStrokeEnd();

private:
    friend class __KisToolPathLocalTool;
};

class KisToolPathFactory : public KoToolFactoryBase
{

public:
    KisToolPathFactory(const QStringList&)
            : KoToolFactoryBase("KisToolPath") {
        setToolTip(i18n("Bezier Curve Tool. Shift-mouseclick ends the curve."));
        setToolType(TOOL_TYPE_SHAPE);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("krita_draw_path"));
        setPriority(7);
    }

    virtual ~KisToolPathFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolPath(canvas);
    }
};



#endif // KIS_TOOL_PATH_H_

