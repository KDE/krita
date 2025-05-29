/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_EXTENDED_MODIFIERS_MAPPER_PLUGIN_INTERFACE_H
#define __KIS_EXTENDED_MODIFIERS_MAPPER_PLUGIN_INTERFACE_H

#include <kritaui_export.h>
#include <Qt>
#include <QObject>
#include <QVector>


class KRITAUI_EXPORT KisExtendedModifiersMapperPluginInterface : public QObject
{
    Q_OBJECT
public:
    typedef QVector<Qt::Key> ExtendedModifiers;

    using QObject::QObject;

    virtual ~KisExtendedModifiersMapperPluginInterface();
    virtual ExtendedModifiers queryExtendedModifiers() = 0;
};

#endif /* __KIS_EXTENDED_MODIFIERS_MAPPER_PLUGIN_INTERFACE_H */