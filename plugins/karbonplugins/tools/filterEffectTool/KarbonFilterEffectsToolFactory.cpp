/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KarbonFilterEffectsToolFactory.h"
#include "KarbonFilterEffectsTool.h"

#include <KoIcon.h>
#include <klocalizedstring.h>

KarbonFilterEffectsToolFactory::KarbonFilterEffectsToolFactory()
    : KoToolFactoryBase("KarbonFilterEffectsTool")
{
    setToolTip(i18n("Filter effects editing"));
    setToolType("karbon,krita");
    setIconName(koIconNameCStr("tool_imageeffects")); // TODO: better icon, e.g. black Fx bad on dark UI
    setPriority(3);
}

KarbonFilterEffectsToolFactory::~KarbonFilterEffectsToolFactory()
{
}

KoToolBase *KarbonFilterEffectsToolFactory::createTool(KoCanvasBase *canvas)
{
    return new KarbonFilterEffectsTool(canvas);
}
