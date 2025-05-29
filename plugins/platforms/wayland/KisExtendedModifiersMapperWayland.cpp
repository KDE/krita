/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisExtendedModifiersMapperWayland.h"

#include <input/kis_extended_modifiers_mapper.h>
#include <krita_container_utils.h>

KisExtendedModifiersMapperWayland::KisExtendedModifiersMapperWayland(QObject *parent, const QVariantList &)
    : KisExtendedModifiersMapperPluginInterface(parent)
{
}

KisExtendedModifiersMapperWayland::ExtendedModifiers KisExtendedModifiersMapperWayland::queryExtendedModifiers() {
    ExtendedModifiers modifiers;
    if (m_watcher.hasKeyboardFocus()) {
        modifiers = m_watcher.pressedKeys();
        KritaUtils::makeContainerUnique(modifiers);
    } else {
        modifiers = KisExtendedModifiersMapper::qtModifiersToQtKeys(m_watcher.modifiers());
    }

    return modifiers;
}