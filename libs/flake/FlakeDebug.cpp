/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "FlakeDebug.h"

const QLoggingCategory &FLAKE_LOG() \
{
    static const QLoggingCategory category("krita.lib.flake", QtInfoMsg);
    return category;
}


