/*
 *  SPDX-FileCopyrightText: 2025 Halla Rempt <halla@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KRITAQMLCOMPONENTSPLUGIN_H
#define KRITAQMLCOMPONENTSPLUGIN_H

#include <QUrl>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class KritaQmlComponentsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    KritaQmlComponentsPlugin(QObject *parent = nullptr);
    void initializeEngine(QQmlEngine *engine, const char *uri) override;
    void registerTypes(const char *uri) override;
};

#endif
