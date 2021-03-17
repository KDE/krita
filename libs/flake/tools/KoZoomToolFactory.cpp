/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoZoomToolFactory.h"
#include "KoZoomTool.h"

#include <KoIcon.h>
#include <klocalizedstring.h>

KoZoomToolFactory::KoZoomToolFactory()
        : KoToolFactoryBase("ZoomTool")
{
    setToolTip(i18n("Zoom"));
    setSection(ToolBoxSection::Navigation);
    setPriority(0);
    setIconName(koIconNameCStr("tool_zoom"));
    setActivationShapeId("flake/always");
}

KoToolBase *KoZoomToolFactory::createTool(KoCanvasBase *canvas)
{
    return new KoZoomTool(canvas);
}
