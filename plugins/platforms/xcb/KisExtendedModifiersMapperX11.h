/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <input/KisExtendedModifiersMapperPluginInterface.h>

#include <QObject>

class KisExtendedModifiersMapperX11
    : public KisExtendedModifiersMapperPluginInterface
{
public:
    KisExtendedModifiersMapperX11(QObject *parent, const QVariantList &);
    ~KisExtendedModifiersMapperX11() override;

    ExtendedModifiers queryExtendedModifiers() override;
};