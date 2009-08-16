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

#include "kis_selection.h"
#include "flake/kis_node_shape.h"
#include "kis_tool.h"

class KisSelectionOptions;
class KoCanvasBase;

class KisToolSelectPath : public KoCreatePathTool
{

    Q_OBJECT

public:
    KisToolSelectPath(KoCanvasBase * canvas);
    virtual ~KisToolSelectPath();

    virtual QWidget * createOptionWidget();

    void addPathShape();
public slots:
    virtual void slotSetAction(int);
    virtual void slotSetSelectionMode(int);
    virtual void activate(bool);

protected:
    /// reimplemented
    virtual QMap<QString, QWidget *> createOptionWidgets();

private:
    KisSelectionOptions * m_optWidget;
    selectionAction m_selectAction;
    selectionMode m_selectionMode;

};

class KisToolSelectPathFactory : public KoToolFactory
{

public:
    KisToolSelectPathFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KisToolSelectPath", i18n("Path Selection")) {
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

