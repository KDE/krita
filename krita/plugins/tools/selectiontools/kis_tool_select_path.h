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
#include <KoToolFactory.h>
#include "kis_tool_select_base.h"

class KisSelectionOptions;
class KoCanvasBase;

class KisToolSelectPath : public KisToolSelectBase
{

    Q_OBJECT

public:
    KisToolSelectPath(KoCanvasBase * canvas);
    virtual ~KisToolSelectPath();

    virtual QWidget * createOptionWidget();

    virtual void paint(QPainter &painter, const KoViewConverter &converter);
    void mousePressEvent(KoPointerEvent *event);
    void mouseDoubleClickEvent(KoPointerEvent *event);
    void mouseMoveEvent(KoPointerEvent *event);
    void mouseReleaseEvent(KoPointerEvent *event);

public slots:
    virtual void activate(bool);
    virtual void deactivate();

private:
    /// reimplemented
    virtual QMap<QString, QWidget *> createOptionWidgets();

    class LokalTool : public KoCreatePathTool {
        friend class KisToolSelectPath;
    public:
        LokalTool(KoCanvasBase * canvas, KisToolSelectPath* selectingTool)
            : KoCreatePathTool(canvas), m_selectingTool(selectingTool) {}
        void addPathShape();
    private:
        KisToolSelectPath* const m_selectingTool;
    };
    LokalTool* const m_lokalTool;

};

class KisToolSelectPathFactory : public KoToolFactory
{

public:
    KisToolSelectPathFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KisToolSelectPath") {
        setToolTip(i18n("Select an area of the image with path."));
        setToolType(TOOL_TYPE_SELECTED);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
        setIcon("tool_path_selection");
        setPriority(58);
    }

    virtual ~KisToolSelectPathFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolSelectPath(canvas);
    }
};



#endif // KIS_TOOL_SELECT_PATH_H_

