/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef BLURPLUGIN_H
#define BLURPLUGIN_H

#include <QObject>
#include <QVariant>

class BlurFilterPlugin : public QObject
{
    Q_OBJECT
public:
    BlurFilterPlugin(QObject *parent, const QVariantList &);
    ~BlurFilterPlugin() override;
};

#endif
