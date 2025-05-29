/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kpluginfactory.h>

#include "KisExtendedModifiersMapperX11.h"

K_PLUGIN_FACTORY_WITH_JSON(KritaPlatformPluginXcbFactory, "kritaplatformxcb.json", registerPlugin<KisExtendedModifiersMapperX11>();)

#include <KritaPlatformPluginXcb.moc>
