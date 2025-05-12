/*
 *  SPDX-FileCopyrightText: 2025 Halla Rempt <halla@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KritaQmlPlugin.h"

#include <QObject>

KritaQmlPlugin::KritaQmlPlugin(QObject *parent)
  : QQmlExtensionPlugin(parent)
{
}

void KritaQmlPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(engine);
    Q_UNUSED(uri);
}

void KritaQmlPlugin::registerTypes(const char *uri)
{
    Q_UNUSED(uri);
}

#include "KritaQmlPlugin.moc"
#include "moc_KritaQmlPlugin.cpp"
