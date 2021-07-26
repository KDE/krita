/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSCREENTONEGENERATORPLUGIN_H
#define KISSCREENTONEGENERATORPLUGIN_H

#include <QObject>

class KisScreentoneGeneratorPlugin : public QObject
{
    Q_OBJECT
public:
    KisScreentoneGeneratorPlugin(QObject *parent, const QVariantList &);
    ~KisScreentoneGeneratorPlugin() override;
};

#endif
