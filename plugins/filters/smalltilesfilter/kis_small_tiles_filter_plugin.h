/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _KIS_SMALL_TILES_FILTER_PLUGIN_H_
#define _KIS_SMALL_TILES_FILTER_PLUGIN_H_

#include <QObject>
#include <QVariant>

class KisSmallTilesFilterPlugin : public QObject
{
    Q_OBJECT
public:
    KisSmallTilesFilterPlugin(QObject *parent, const QVariantList &);
    ~KisSmallTilesFilterPlugin() override;
};

#endif
