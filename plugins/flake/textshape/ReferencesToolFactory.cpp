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

#include "ReferencesToolFactory.h"
#include "ReferencesTool.h"
#include "TextShape.h"

#include <KoCanvasBase.h>
#include <KoIcon.h>

#include <klocalizedstring.h>

ReferencesToolFactory::ReferencesToolFactory()
    : KoToolFactoryBase("ReferencesTool")
{
    setToolTip(i18n("References"));
    setToolType("calligrawords,calligraauthor");
    setIconName(koIconNameCStr("tool_references"));
    setPriority(20);
    setActivationShapeId(TextShape_SHAPEID);
}

ReferencesToolFactory::~ReferencesToolFactory()
{
}

KoToolBase *ReferencesToolFactory::createTool(KoCanvasBase *canvas)
{
    return new ReferencesTool(canvas);
}
