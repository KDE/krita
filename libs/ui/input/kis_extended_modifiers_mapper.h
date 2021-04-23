/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_EXTENDED_MODIFIERS_MAPPER_H
#define __KIS_EXTENDED_MODIFIERS_MAPPER_H

#include <Qt>
#include <QVector>
#include <QScopedPointer>

#include <kritaui_export.h>

class QKeyEvent;
class KisShortcutMatcher;

class KRITAUI_EXPORT KisExtendedModifiersMapper
{
public:
    KisExtendedModifiersMapper();
    ~KisExtendedModifiersMapper();

    typedef QVector<Qt::Key> ExtendedModifiers;

    ExtendedModifiers queryExtendedModifiers();
    Qt::KeyboardModifiers queryStandardModifiers();

    static Qt::Key workaroundShiftAltMetaHell(const QKeyEvent *keyEvent);

#ifdef Q_OS_MACOS
    static void setLocalMonitor(bool activate, KisShortcutMatcher *matcher = 0);
#endif

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif /* __KIS_EXTENDED_MODIFIERS_MAPPER_H */
