/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Frederic Coiffier <fcoiffie@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef KIS_LEVEL_FILTER_PLUGIN_H
#define KIS_LEVEL_FILTER_PLUGIN_H

#include <QObject>
#include <QVariant>

class KisLevelFilterPlugin : public QObject
{
    Q_OBJECT
public:
    KisLevelFilterPlugin(QObject *parent, const QVariantList &);
    ~KisLevelFilterPlugin() override;
};

#endif
