/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_EXIF_PLUGIN_H_
#define _KIS_EXIF_PLUGIN_H_

#include <QObject>

class KisExifPlugin : public QObject
{
public:
    KisExifPlugin(QObject *parent, const QVariantList &);
    ~KisExifPlugin() override;
};

#endif // _KIS_EXIF_PLUGIN_H_
