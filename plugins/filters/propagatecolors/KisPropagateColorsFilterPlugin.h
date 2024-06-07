/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PROPAGATE_COLORS_FILTER_PLUGIN_H
#define KIS_PROPAGATE_COLORS_FILTER_PLUGIN_H

#include <QObject>
#include <QVariant>

class KisPropagateColorsFilterPlugin : public QObject
{
    Q_OBJECT
public:
    KisPropagateColorsFilterPlugin(QObject *parent, const QVariantList &);
    ~KisPropagateColorsFilterPlugin() override;
};

#endif
