/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _KIS_OILPAINT_FILTER_PLUGIN_H_
#define _KIS_OILPAINT_FILTER_PLUGIN_H_

#include <QObject>
#include <QVariant>

class KisOilPaintFilterPlugin : public QObject
{
    Q_OBJECT
public:
    KisOilPaintFilterPlugin(QObject *parent, const QVariantList &);
    ~KisOilPaintFilterPlugin() override;
};

#endif
