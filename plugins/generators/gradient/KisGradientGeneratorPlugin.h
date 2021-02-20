/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISGRADIENTGENERATORPLUGIN_H
#define KISGRADIENTGENERATORPLUGIN_H

#include <QObject>

class KisGradientGeneratorPlugin : public QObject
{
    Q_OBJECT
public:
    KisGradientGeneratorPlugin(QObject *parent, const QVariantList &);
    ~KisGradientGeneratorPlugin() override;
};

#endif
