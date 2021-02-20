/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef _EXTENSIONS_PLUGIN_H_
#define _EXTENSIONS_PLUGIN_H_

#include <QObject>
#include <QVariant>

class ExtensionsPlugin : public QObject
{
    Q_OBJECT
public:
    ExtensionsPlugin(QObject *parent, const QVariantList &);
    ~ExtensionsPlugin() override;

};

#endif
