/*
 *  SPDX-FileCopyrightText: 2025 Halla Rempt <halla@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KRITAQMLPLUGIN_H
#define KRITAQMLPLUGIN_H

#include <QUrl>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class KritaQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    KritaQmlPlugin(QObject *parent = nullptr);
    void initializeEngine(QQmlEngine *engine, const char *uri) override;
    void registerTypes(const char *uri) override;
};

#endif
