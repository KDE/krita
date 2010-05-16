/* This file is part of the KDE project
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>
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

#include "ChangeTrackingToolFactory.h"
#include "ChangeTrackingTool.h"
#include "TextShape.h"

#include <KoCanvasBase.h>
#include <KoShapeController.h>
#include <KoResourceManager.h>

#include <klocale.h>

ChangeTrackingToolFactory::ChangeTrackingToolFactory(QObject *parent)
        : KoToolFactoryBase(parent, "ChangeTrackingToolFactory_ID")
{
    setToolTip(i18n("Change Tracking tool"));
    setToolType(dynamicToolType());
    setIcon("tool-changetracking");
    setPriority(3);
    setActivationShapeId(TextShape_SHAPEID);
}

ChangeTrackingToolFactory::~ChangeTrackingToolFactory()
{
}

KoToolBase * ChangeTrackingToolFactory::createTool(KoCanvasBase *canvas)
{
    return new ChangeTrackingTool(canvas);
}

bool ChangeTrackingToolFactory::canCreateTool(KoCanvasBase* canvas) const
{
    if (canvas->shapeController()->resourceManager() && canvas->shapeController()->resourceManager()->hasResource(KoText::ChangeTracker))
        return true;
    return false;
}

#include <ChangeTrackingToolFactory.moc>
