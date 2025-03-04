/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "Plugin.h"

#include <kpluginfactory.h>

#include <QQmlEngine>

#include <KoShapeRegistry.h>
#include <KoToolRegistry.h>

#include <KisStaticInitializer.h>
#include "SvgTextToolFactory.h"

#include "glyphpalette/GlyphPaletteProxyModel.h"

K_PLUGIN_FACTORY_WITH_JSON(PluginFactory, "krita_tool_svgtext.json", registerPlugin<Plugin>();)

KIS_DECLARE_STATIC_INITIALIZER {
    //qmlRegisterType<SvgTextLabel>("org.krita.tools.text", 1, 0, "SvgTextLabel");
    qmlRegisterType<GlyphPaletteProxyModel>("org.krita.tools.text", 1, 0, "GlyphPaletteProxyModel");
}
Plugin::Plugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoToolRegistry::instance()->add(new SvgTextToolFactory());
}

#include <Plugin.moc>

