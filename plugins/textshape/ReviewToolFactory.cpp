/* This file is part of the KDE project
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_calligra@gadz.org>
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

#include "ReviewToolFactory.h"
#include "ReviewTool.h"
#include "TextShape.h"

#include <KoCanvasBase.h>
#include <KoShapeController.h>
#include <KoDocumentResourceManager.h>

#include <KoIcon.h>
#include <klocale.h>

ReviewToolFactory::ReviewToolFactory()
        : KoToolFactoryBase("ReviewToolFactory_ID")
{
    setToolTip(i18n("Review"));
    setToolType(dynamicToolType()+",calligrawords");
    setIconName(koIconNameCStr("tool_review"));
    setPriority(30);
    setActivationShapeId(TextShape_SHAPEID);
}

ReviewToolFactory::~ReviewToolFactory()
{
}

KoToolBase *ReviewToolFactory::createTool(KoCanvasBase *canvas)
{
    return new ReviewTool(canvas);
}

bool ReviewToolFactory::canCreateTool(KoCanvasBase* canvas) const
{
    if (canvas->shapeController()->resourceManager() && canvas->shapeController()->resourceManager()->hasResource(KoText::ChangeTracker))
        return true;
    return false;
}
