/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_RAINDROPS_FILTER_PLUGIN_H_
#define _KIS_RAINDROPS_FILTER_PLUGIN_H_

#include <QObject>
#include <QVariant>

class KisRainDropsFilterPlugin : public QObject
{
    Q_OBJECT
public:
    KisRainDropsFilterPlugin(QObject *parent, const QVariantList &);
    ~KisRainDropsFilterPlugin() override;
};

#endif
