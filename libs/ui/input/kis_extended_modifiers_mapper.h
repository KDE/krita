/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    /**
     * Query the native platform API for keys.
     *
     * A word of caution: using the result of this to modify KisShortcutMatcher's state (directly or indirectly)
     * can result in keys becoming "stuck" whenever we observe key status through this function, especially when
     * the application does not have focus.
     * On Windows in particular, [Windows Key]+D or [Windows Key]+[0~9] are prone to this behavior.
     * \see KisInputManager::Private::fixShortcutMatcherModifiersState
     */
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
