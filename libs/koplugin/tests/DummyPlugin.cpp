/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "DummyPlugin.h"

#include <kpluginfactory.h>

K_PLUGIN_FACTORY_WITH_JSON(DummyPluginFactory, DUMMY_PLUGIN_JSON_FILE, registerPlugin<DummyPlugin>();)

DummyPlugin::DummyPlugin(QObject *parent, const QVariantList &)
    : DummyTrivialInterface(parent)
{
}

#include <DummyPlugin.moc>
#include <moc_DummyPlugin.cpp>
