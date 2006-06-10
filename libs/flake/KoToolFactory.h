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
#include <QObject>

/**
 * A factory for KoTool objects.
 * This abstract class is a baseclass for real factories; each class inheriting from this one
 * will be the creator of one specific tool class. So there is one toolFactory for each type 
 * of tool.
 */
class FLAKE_EXPORT KoToolFactory : public QObject {
    Q_OBJECT

public:
    /// constructor
    KoToolFactory();
    virtual ~KoToolFactory();

    /// instanciate a new tool
    virtual KoTool * createTool(KoCanvasBase *canvas) = 0;
    /**
     * return the ID for the tool this factory is associated with.
     * @return the ID for the tool this factory is associated with.
     */
    virtual KoID id() = 0;
    /// The priority of this tool in its section in the toolbox
    virtual quint32 priority() const = 0;
    /// the type of tool, used to group tools in the toolbox
    virtual const QString& toolType() const = 0;
    /// return a translated tooltip Text
    virtual const QString& tooltipText() const = 0;
    /// return an icon for this tool
    virtual const QPixmap& icon() const = 0;
    /// The shape ID the tool is associated with, or 0 when the tool is a generic tool
    virtual KoID activationShapeId() const = 0;
};

#endif

