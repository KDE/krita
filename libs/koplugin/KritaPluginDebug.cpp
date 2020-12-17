/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KritaPluginDebug.h"

const QLoggingCategory &PLUGIN_LOG() \
{
    static const QLoggingCategory category("krita.lib.plugin", QtInfoMsg);
    return category;
}


