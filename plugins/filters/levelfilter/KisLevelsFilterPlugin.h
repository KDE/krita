/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Frederic Coiffier <fcoiffie@gmail.com>
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_LEVELS_FILTER_PLUGIN_H
#define KIS_LEVELS_FILTER_PLUGIN_H

#include <QObject>
#include <QVariant>

class KisLevelsFilterPlugin : public QObject
{
    Q_OBJECT
public:
    KisLevelsFilterPlugin(QObject *parent, const QVariantList &);
    ~KisLevelsFilterPlugin() override;
};

#endif
