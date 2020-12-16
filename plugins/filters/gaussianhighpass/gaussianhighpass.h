/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2019 Miguel Lopez <reptillia39@live.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _GAUSSIANHIGHPASS_PLUGIN_H_
#define _GAUSSIANHIGHPASS_PLUGIN_H_

#include <QObject>
#include <QVariant>

class GaussianHighPassPlugin : public QObject
{
    Q_OBJECT
public:
    GaussianHighPassPlugin(QObject *parent, const QVariantList &);
    ~GaussianHighPassPlugin() override;
};

#endif
