/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "KoPencilToolFactory.h"
#include "KoPencilTool.h"

#include <KoToolRegistry.h>

#include <KoIcon.h>
#include <klocale.h>
#include <kdebug.h>

KoPencilToolFactory::KoPencilToolFactory()
        : KoToolFactoryBase("KoPencilTool")
{
    setToolTip(i18n("Freehand Path Drawing Tool"));
    setToolType("karbon, krita");
    setIconName(koIconNameCStr("draw-freehand"));
    setPriority(3);
    setActivationShapeId("flake/edit");
}

KoPencilToolFactory::~KoPencilToolFactory()
{
}

KoToolBase * KoPencilToolFactory::createTool(KoCanvasBase *canvas)
{
    return new KoPencilTool(canvas);
}

