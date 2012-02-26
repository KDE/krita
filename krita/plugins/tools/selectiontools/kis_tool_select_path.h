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

#include <KoPointerEvent.h>

class KoCanvasBase;
class KoLineBorder;

#define REDIRECT_EVENT(name, EventType)         \
    void name(EventType *e) {                   \
        m_localTool->name(e);                   \
        if (!e->isAccepted()) {                 \
            KisToolSelectBase::name(e);         \
        }                                       \
    }

class KisToolSelectPath : public KisToolSelectBase
{
    Q_OBJECT

public:
    KisToolSelectPath(KoCanvasBase * canvas);
    virtual ~KisToolSelectPath();

    void paint(QPainter& gc, const KoViewConverter &converter) {m_localTool->paint(gc, converter);}
    REDIRECT_EVENT(mousePressEvent, KoPointerEvent);
    REDIRECT_EVENT(mouseMoveEvent, KoPointerEvent);
    REDIRECT_EVENT(mouseReleaseEvent, KoPointerEvent);
    REDIRECT_EVENT(keyPressEvent, QKeyEvent);
    REDIRECT_EVENT(keyReleaseEvent, QKeyEvent);
    void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes) {m_localTool->activate(toolActivation, shapes); KisToolSelectBase::activate(toolActivation, shapes);}
    void deactivate() {m_localTool->deactivate(); KisToolSelectBase::deactivate();}

private:
    /// reimplemented
    virtual QList<QWidget *> createOptionWidgets();

    class LocalTool : public KoCreatePathTool {
        friend class KisToolSelectPath;
    public:
        LocalTool(KoCanvasBase * canvas, KisToolSelectPath* selectingTool);
        virtual void paintPath(KoPathShape &path, QPainter &painter, const KoViewConverter &converter);
        virtual void addPathShape(KoPathShape* pathShape);
    private:
        KisToolSelectPath* const m_selectionTool;
        KoLineBorder* m_borderBackup;
    };
    LocalTool* const m_localTool;

};

class KisToolSelectPathFactory : public KoToolFactoryBase
{

public:
    KisToolSelectPathFactory(const QStringList&)
            : KoToolFactoryBase("KisToolSelectPath") {
        setToolTip(i18n("Select an area of the image with path."));
        setToolType(TOOL_TYPE_SELECTED);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIcon("tool_path_selection");
        setPriority(58);
    }

    virtual ~KisToolSelectPathFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolSelectPath(canvas);
    }
};



#endif // KIS_TOOL_SELECT_PATH_H_

