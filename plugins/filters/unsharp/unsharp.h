/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _UNSHARP_PLUGIN_H_
#define _UNSHARP_PLUGIN_H_

#include <QObject>
#include <QVariant>

class UnsharpPlugin : public QObject
{
    Q_OBJECT
public:
    UnsharpPlugin(QObject *parent, const QVariantList &);
    ~UnsharpPlugin() override;
};

#endif
