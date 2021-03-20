/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SvgTextToolFactory.h"

#include "KoSvgTextShape.h"
#include "SvgTextTool.h"

#include <KoIcon.h>
#include <klocalizedstring.h>

SvgTextToolFactory::SvgTextToolFactory()
    : KoToolFactoryBase("SvgTextTool")
{
    setToolTip(i18n("SVG Text Tool"));
    setIconName(koIconNameCStr("draw-text"));
    setSection(mainToolType());
    setPriority(1);
    setActivationShapeId(QString("flake/always,%1").arg(KoSvgTextShape_SHAPEID));
}

SvgTextToolFactory::~SvgTextToolFactory()
{
}

KoToolBase *SvgTextToolFactory::createTool(KoCanvasBase *canvas)
{
    return new SvgTextTool(canvas);
}

