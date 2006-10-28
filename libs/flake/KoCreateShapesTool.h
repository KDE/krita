/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOCREATESHAPESTOOL_H
#define KOCREATESHAPESTOOL_H

#include "KoInteractionTool.h"
#include "KoShapeController.h"
#include "KoShapeFactory.h"

#include <koffice_export.h>

#include <QString>

class KoCanvasBase;
class KoShapeControllerBase;

#define KoCreateShapesTool_ID "CreateShapesTool"

/**
 * A tool to create shapes with.
 */
class FLAKE_EXPORT KoCreateShapesTool : public KoShapeController, public KoInteractionTool
{
public:
    /**
     * Create a new tool; typically not called by applications, only by the KoToolManager
     * @param canvas the canvas this tool works for.
     */
    KoCreateShapesTool( KoCanvasBase *canvas);
    /// destructor
    ~KoCreateShapesTool() {};
    void mouseReleaseEvent( KoPointerEvent *event );

    void paint( QPainter &painter, KoViewConverter &converter );
};

#endif
