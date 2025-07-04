/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_extended_modifiers_mapper.h"
#include "KisExtendedModifiersMapperPluginInterface.h"

#include <KisApplication.h>
#include <QKeyEvent>

#include <krita_container_utils.h>

#ifdef Q_OS_MACOS

#include "kis_extended_modifiers_mapper_osx.h"

#endif /* Q_OS_MACOS */

#ifdef Q_OS_WIN

#include <windows.h>
#include <commctrl.h>
#include <winuser.h>

#include "krita_container_utils.h"
#include "kis_config.h"


QVector<Qt::Key> queryPressedKeysWin()
{
    QVector<Qt::Key> result;
    BYTE vkeys[256];

    KisConfig cfg(true);

    const int maxFunctionKey = cfg.ignoreHighFunctionKeys() ? VK_F12 : VK_F24;

    if (GetKeyboardState(vkeys)) {
        for (int i = 0; i < 256; i++) {
            if (vkeys[i] & 0x80) {
                if (i == VK_SHIFT) {
                    result << Qt::Key_Shift;
                } else if (i == VK_CONTROL) {
                    result << Qt::Key_Control;
                } else if (i == VK_MENU) {
                    result << Qt::Key_Alt;
                } else if (i == VK_LWIN || i == VK_RWIN) {
                    result << Qt::Key_Meta;
                } else if (i == VK_SPACE) {
                    result << Qt::Key_Space;
                } else if (i >= 0x30 && i <= 0x39) {
                    result << static_cast<Qt::Key>(Qt::Key_0 + i - 0x30);
                } else if (i >= 0x41 && i <= 0x5A) {
                    result << static_cast<Qt::Key>(Qt::Key_A + i - 0x41);
                } else if (i >= 0x60 && i <= 0x69) {
                    result << static_cast<Qt::Key>(Qt::Key_0 + i - 0x60);
                } else if (i >= VK_F1 && i <= maxFunctionKey) {
                    result << static_cast<Qt::Key>(Qt::Key_F1 + i - VK_F1);
                }
            }
        }
    }

    KritaUtils::makeContainerUnique(result);

    return result;
}

#endif /* Q_OS_WIN */

struct KisExtendedModifiersMapper::Private
{
};

KisExtendedModifiersMapper::KisExtendedModifiersMapper()
    : m_d(new Private)
{
}

KisExtendedModifiersMapper::~KisExtendedModifiersMapper()
{
}

#ifdef Q_OS_MACOS
void KisExtendedModifiersMapper::setLocalMonitor(bool activate, KisShortcutMatcher *matcher)
{
    Q_UNUSED(matcher);
    activateLocalMonitor(activate);
}
#endif

KisExtendedModifiersMapper::ExtendedModifiers
KisExtendedModifiersMapper::queryExtendedModifiers()
{
    KisExtendedModifiersMapperPluginInterface *plugin =
        static_cast<KisApplication*>(qApp)->extendedModifiersPluginInterface();

    if (plugin) {
        return plugin->queryExtendedModifiers();
    }

    ExtendedModifiers modifiers;

#if defined Q_OS_WIN
    modifiers = queryPressedKeysWin();
#elif defined Q_OS_MACOS
    modifiers = queryPressedKeysMac();
#else
    modifiers = qtModifiersToQtKeys(queryStandardModifiers());
#endif

    return modifiers;
}

Qt::KeyboardModifiers KisExtendedModifiersMapper::queryStandardModifiers()
{
    return QApplication::queryKeyboardModifiers();
}

Qt::Key KisExtendedModifiersMapper::workaroundShiftAltMetaHell(const QKeyEvent *keyEvent)
{
    Qt::Key key = (Qt::Key)keyEvent->key();

    if (keyEvent->key() == Qt::Key_Meta &&
        keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {

        key = Qt::Key_Alt;
    }

    return key;
}

KisExtendedModifiersMapper::ExtendedModifiers KisExtendedModifiersMapper::qtModifiersToQtKeys(Qt::KeyboardModifiers standardModifiers)
{
    ExtendedModifiers modifiers;

    if (standardModifiers & Qt::ShiftModifier) {
        modifiers << Qt::Key_Shift;
    }

    if (standardModifiers & Qt::ControlModifier) {
        modifiers << Qt::Key_Control;
    }

    if (standardModifiers & Qt::AltModifier) {
        modifiers << Qt::Key_Alt;
    }

    if (standardModifiers & Qt::MetaModifier) {
        modifiers << Qt::Key_Meta;
    }

    return modifiers;
};