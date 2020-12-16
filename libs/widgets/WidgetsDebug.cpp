/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "WidgetsDebug.h"

const QLoggingCategory &WIDGETS_LOG() \
{
    static const QLoggingCategory category("krita.lib.widgets", QtInfoMsg);
    return category;
}


