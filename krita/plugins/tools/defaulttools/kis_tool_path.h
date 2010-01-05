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
#include <KoToolFactory.h>

#include "flake/kis_node_shape.h"
#include "kis_tool.h"

class KisSelectionOptions;
class KoCanvasBase;

class KisToolPath : public KoCreatePathTool
{

    Q_OBJECT

public:
    KisToolPath(KoCanvasBase * canvas);
    virtual ~KisToolPath();

    void addPathShape();

};

class KisToolPathFactory : public KoToolFactory
{

public:
    KisToolPathFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KisToolPath") {
        setToolTip(i18n("Draw a path."));
        setToolType(TOOL_TYPE_SHAPE);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
        setIcon("krita_draw_path");
        setPriority(7);
    }

    virtual ~KisToolPathFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolPath(canvas);
    }
};



#endif // KIS_TOOL_PATH_H_

