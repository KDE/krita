/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_EXTENDED_MODIFIERS_MAPPER_WAYLAND_H
#define KIS_EXTENDED_MODIFIERS_MAPPER_WAYLAND_H

#include <input/KisExtendedModifiersMapperPluginInterface.h>
#include <KisWaylandKeyboardWatcher.h>

class KisExtendedModifiersMapperWayland : public KisExtendedModifiersMapperPluginInterface
{
    Q_OBJECT
public:
    KisExtendedModifiersMapperWayland(QObject *parent, const QVariantList &);
    ExtendedModifiers queryExtendedModifiers() override;

private:
    KisWaylandKeyboardWatcher m_watcher;
};

#endif // KIS_EXTENDED_MODIFIERS_MAPPER_WAYLAND_H