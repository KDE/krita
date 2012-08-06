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
#include <KoIcon.h>

class KisSelectionOptions;
class KoCanvasBase;

class KisToolPath : public KisToolShape
{

    Q_OBJECT

public:
    KisToolPath(KoCanvasBase * canvas);
    virtual ~KisToolPath();

    virtual void paint(QPainter &painter, const KoViewConverter &converter);
    void mousePressEvent(KoPointerEvent *event);
    void mouseDoubleClickEvent(KoPointerEvent *event);
    void mouseMoveEvent(KoPointerEvent *event);
    void mouseReleaseEvent(KoPointerEvent *event);
    
    void addPathShape(KoPathShape* pathShape);

public slots:
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    virtual void deactivate();

private:
    /// reimplemented
    virtual QList<QWidget *> createOptionWidgets();

    class LocalTool : public KoCreatePathTool {
        friend class KisToolPath;
    public:
        LocalTool(KoCanvasBase * canvas, KisToolPath* selectingTool);
        virtual void paintPath(KoPathShape &path, QPainter &painter, const KoViewConverter &converter);
        virtual void addPathShape(KoPathShape* pathShape);
    private:
        KisToolPath* const m_parentTool;
    };
    LocalTool* const m_localTool;

};

class KisToolPathFactory : public KoToolFactoryBase
{

public:
    KisToolPathFactory(const QStringList&)
            : KoToolFactoryBase("KisToolPath") {
        setToolTip(i18n("Draw a path. Shift-mouseclick ends the path."));
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

