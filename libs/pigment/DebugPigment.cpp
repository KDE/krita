/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "DebugPigment.h"
const QLoggingCategory &PIGMENT_log() \
{
    static const QLoggingCategory category("krita.lib.pigment", QtInfoMsg);
    return category;
}


