/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KO_TOOL_FACTORY_H
#define KO_TOOL_FACTORY_H

#include "KoTool.h"
#include <KoID.h>
#include <koffice_export.h>

#include <klocale.h>
#include <QObject>

class FLAKE_EXPORT KoToolFactory : public QObject {
    Q_OBJECT

public:
    KoToolFactory();
    virtual ~KoToolFactory();

    /// instanciate a new tool
    virtual KoTool * createTool(KoCanvasBase *canvas) = 0;
    virtual KoID id() = 0;
    /// The priority of this tool in its section in the toolbox
    virtual quint32 priority() const = 0;
    /// the type of tool, used to group tools in the toolbox
    virtual QString toolType() const = 0;
    /// return a translated tooltip Text
    virtual QString tooltipText() const = 0;
    // getter for icon?
    /// The shape ID the tool is associated with, or 0 when the tool is a generic tool
    virtual KoID* activationShapeId() const = 0;
};

#endif

