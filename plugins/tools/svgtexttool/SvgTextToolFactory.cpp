/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SvgTextToolFactory.h"

#include "KoSvgTextShape.h"
#include "SvgTextTool.h"
#include "SvgTextShortCuts.h"

#include <KoIcon.h>
#include <klocalizedstring.h>
#include <kis_action_registry.h>

SvgTextToolFactory::SvgTextToolFactory()
    : KoToolFactoryBase("SvgTextTool")
{
    setToolTip(i18n("SVG Text Tool"));
    setIconName(koIconNameCStr("draw-text"));
    setSection(ToolBoxSection::Main);
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

QList<QAction *> SvgTextToolFactory::createActionsImpl()
{
    QList<QAction *> actions;
    Q_FOREACH(const QString name, SvgTextShortCuts::possibleActions()) {
        actions << KisActionRegistry::instance()->makeQAction(name, this);
    }
    actions << KisActionRegistry::instance()->makeQAction("svg_insert_special_character", this);
    actions << KisActionRegistry::instance()->makeQAction("svg_paste_rich_text", this);
    actions << KisActionRegistry::instance()->makeQAction("svg_paste_plain_text", this);
    actions << KisActionRegistry::instance()->makeQAction("text_type_preformatted", this);
    actions << KisActionRegistry::instance()->makeQAction("text_type_pre_positioned", this);
    actions << KisActionRegistry::instance()->makeQAction("text_type_inline_wrap", this);
    return actions;
}

