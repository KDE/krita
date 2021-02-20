/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2016 Spencer Brown <sbrown655@gmail.com>
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRADIENT_MAP_FILTER_PLUGIN_H
#define KIS_GRADIENT_MAP_FILTER_PLUGIN_H

#include <QObject>

class KisGradientMapFilterPlugin : public QObject
{
    Q_OBJECT
public:
    KisGradientMapFilterPlugin(QObject *parent, const QVariantList &);
    ~KisGradientMapFilterPlugin() override;
};

#endif
