/*
 *  SPDX-FileCopyrightText: 2025 Halla Rempt <halla@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KritaQmlComponentsPlugin.h"

#include <QObject>
#include <KisStaticInitializer.h>

#include <SvgTextLabel.h>
#include <KisNumParser.h>
#include <KisCubicCurveQMLWrapper.h>
#include <KisTheme.h>
#include <KisFontFunctions.h>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#error "This plugin implementation is to be used with Qt5 only!\n"
       "Qt6 should generate it itself with GENERATE_PLUGIN_SOURCE!"
#endif

KIS_DECLARE_STATIC_INITIALIZER {
    qmlRegisterModule("org.krita.components", 1, 0);
}

KritaQmlComponentsPlugin::KritaQmlComponentsPlugin(QObject *parent)
  : QQmlExtensionPlugin(parent)
{
}

void KritaQmlComponentsPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(engine);
    Q_UNUSED(uri);
}

void KritaQmlComponentsPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<SvgTextLabel>(uri, 1, 0, "SvgTextLabel");
    qmlRegisterType<KisNumParser>(uri, 1, 0, "KisNumParser");
    qmlRegisterType<KisCubicCurveQml>(uri, 1, 0, "KisCubicCurve");
    qmlRegisterType<KisTheme>(uri, 1, 0, "Theme");
    qmlRegisterUncreatableType<KisThemeColorGroup>(uri, 1, 0, "ThemeColorGroup", "Use Theme instead");
    qmlRegisterType<KisFontFunctions>(uri, 1, 0, "FontFunctions");
}

#include "moc_KritaQmlComponentsPlugin.cpp"
