/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef WIDGETUTILS_DEBUG_H_
#define WIDGETUTILS_DEBUG_H_

#include <QDebug>
#include <QLoggingCategory>

extern const QLoggingCategory &KRITAWIDGETUTILS_LOG();

#define debugWidgetUtils qCDebug(KRITAWIDGETUTILS_LOG)
#define warnWidgetUtils qCWarning(KRITAWIDGETUTILS_LOG)
#define errorWidgetUtils qCCritical(KRITAWIDGETUTILS_LOG)

#endif
