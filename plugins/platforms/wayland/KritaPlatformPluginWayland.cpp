/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kpluginfactory.h>

#include <KisExtendedModifiersMapperWayland.h>

K_PLUGIN_FACTORY_WITH_JSON(KritaPlatformPluginWaylandFactory, "kritaplatformwayland.json", registerPlugin<KisExtendedModifiersMapperWayland>();)

#include <KritaPlatformPluginWayland.moc>
