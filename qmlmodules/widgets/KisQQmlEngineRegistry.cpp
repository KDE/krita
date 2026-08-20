/*
 * SPDX-FileCopyrightText: 2026 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisQQmlEngineRegistry.h"

#include <KoResourcePaths.h>
#include <QQmlFileSelector>
#include <QFileSelector>
#include <QQuickStyle>

Q_GLOBAL_STATIC(KisQQmlEngineRegistry, s_instance)

struct KisQQmlEngineRegistry::Private {
    QQmlEngine *qQuickWidgetEngine = nullptr;
};

KisQQmlEngineRegistry::KisQQmlEngineRegistry(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

KisQQmlEngineRegistry::~KisQQmlEngineRegistry()
{

}

KisQQmlEngineRegistry *KisQQmlEngineRegistry::instance()
{
    return s_instance;
}

QQmlEngine *KisQQmlEngineRegistry::qQuickWidgetEngine()
{
    if (!d->qQuickWidgetEngine) {
        // TODO: parent?
        d->qQuickWidgetEngine = new QQmlEngine();
        d->qQuickWidgetEngine->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib/qml/");
        d->qQuickWidgetEngine->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib64/qml/");

        d->qQuickWidgetEngine->addPluginPath(KoResourcePaths::getApplicationRoot() + "/lib/qml/");
        d->qQuickWidgetEngine->addPluginPath(KoResourcePaths::getApplicationRoot() + "/lib64/qml/");

        QQmlFileSelector* selector = new QQmlFileSelector(d->qQuickWidgetEngine);
        QStringList extraSelectors;
        /*
         * This allows for Style specific components, which it'll load from
         * a "+StyleName" folder.
         */
        extraSelectors << QQuickStyle::name();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        extraSelectors << "qt6";
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        extraSelectors << "qt5";
#endif
        /**
         * WARNING: The following selector is *only* for KisQQuickWidget. Do not
         * copy it to other engines.
         */
        extraSelectors << "qquickwidget";

        selector->setExtraSelectors(extraSelectors);
    }
    return d->qQuickWidgetEngine;
}
