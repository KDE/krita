/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_XMP_PLUGIN_H_
#define _KIS_XMP_PLUGIN_H_

#include <QObject>

class KisXmpPlugin : public QObject
{
public:
    KisXmpPlugin(QObject *parent, const QVariantList &);
    ~KisXmpPlugin() override;
};

#endif // _KIS_IPTC_PLUGIN_H_
