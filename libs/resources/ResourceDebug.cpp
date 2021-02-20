/*
 *  SPDX-FileCopyrightText: 2020 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ResourceDebug.h"

const QLoggingCategory &RESOURCE_LOG() \
{
    static const QLoggingCategory category("krita.lib.resource", QtInfoMsg);
    return category;
}


