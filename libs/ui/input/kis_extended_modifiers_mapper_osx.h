/*
 *  SPDX-FileCopyrightText: 2020 Ivan SantaMaria <ghevan@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_EXTENDED_MODIFIERS_MAPPER_OSX_H
#define KIS_EXTENDED_MODIFIERS_MAPPER_OSX_H

#include <QApplication>
#include <QKeyEvent>

#include "kis_shortcut_matcher.h"

/*
 * global Monitor captures events when other
 * applications have focus.
 *
 * */
void activateGlobalMonitor(bool activate);

void activateLocalMonitor(bool activate, KisShortcutMatcher &matcher);

QVector<Qt::Key> queryPressedKeysMac();

#endif // KIS_EXTENDED_MODIFIERS_MAPPER_OSX_H
