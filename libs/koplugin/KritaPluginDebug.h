/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef FLAKE_DEBUG_H_
#define FLAKE_DEBUG_H_

#include <QDebug>
#include <QLoggingCategory>
#include "kritaplugin_export.h"

extern const KRITAPLUGIN_EXPORT QLoggingCategory &PLUGIN_LOG();

#define debugPlugin qCDebug(PLUGIN_LOG)
#define warnPlugin qCWarning(PLUGIN_LOG)
#define errorPlugin qCCritical(PLUGIN_LOG)

#endif
