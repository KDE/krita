/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_IPTC_PLUGIN_H_
#define _KIS_IPTC_PLUGIN_H_

#include <QObject>

class KisIptcPlugin : public QObject
{
public:
    KisIptcPlugin(QObject *parent, const QVariantList &);
    ~KisIptcPlugin() override;
};

#endif // _KIS_IPTC_PLUGIN_H_
