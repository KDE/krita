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

#ifndef KIS_TOOL_SELECT_PATH_H_
#define KIS_TOOL_SELECT_PATH_H_

#include <KoCreatePathTool.h>
#include <KoToolFactoryBase.h>
#include "kis_tool_select_base.h"
#include "kis_delegated_tool.h"

#include <KoIcon.h>

class KoCanvasBase;
class KoShapeStroke;
class KisToolSelectPath;


class __KisToolSelectPathLocalTool : public KoCreatePathTool {
public:
    __KisToolSelectPathLocalTool(KoCanvasBase * canvas, KisToolSelectPath* parentTool);
    virtual void paintPath(KoPathShape &path, QPainter &painter, const KoViewConverter &converter);
    virtual void addPathShape(KoPathShape* pathShape);

    using KoCreatePathTool::createOptionWidgets;
    using KoCreatePathTool::endPathWithoutLastPoint;
    using KoCreatePathTool::endPath;
    using KoCreatePathTool::cancelPath;

private:
    KisToolSelectPath* const m_selectionTool;
};

struct __KisToolSelectBaseWrapper : public KisToolSelectBase {
    __KisToolSelectBaseWrapper(KoCanvasBase *canvas,
                               const QCursor &cursor)
        : KisToolSelectBase(canvas, cursor, i18n("Path Selection"))
    {
    }
};

typedef KisDelegatedTool<__KisToolSelectBaseWrapper,
                         __KisToolSelectPathLocalTool,
                         DeselectShapesActivationPolicy> DelegatedSelectPathTool;

class KisToolSelectPath : public DelegatedSelectPathTool
{
    Q_OBJECT

public:
    KisToolSelectPath(KoCanvasBase * canvas);
    void mousePressEvent(KoPointerEvent* event);

protected:
    void requestStrokeCancellation();
    void requestStrokeEnd();

    friend class __KisToolSelectPathLocalTool;
    QList<QPointer<QWidget> > createOptionWidgets();
};

class KisToolSelectPathFactory : public KoToolFactoryBase
{

public:
    KisToolSelectPathFactory(const QStringList&)
            : KoToolFactoryBase("KisToolSelectPath") {
        setToolTip(i18n("Bezier Curve Selection Tool"));
        setToolType(TOOL_TYPE_SELECTED);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("tool_path_selection"));
        setPriority(58);
    }

    virtual ~KisToolSelectPathFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolSelectPath(canvas);
    }
};



#endif // KIS_TOOL_SELECT_PATH_H_

