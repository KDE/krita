/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KARBONTOOLSPLUGIN_H_
#define _KARBONTOOLSPLUGIN_H_

#include <QObject>
#include <QVariant>

class KarbonToolsPlugin : public QObject
{
    Q_OBJECT

public:
    KarbonToolsPlugin(QObject *parent,  const QVariantList &);
    ~KarbonToolsPlugin() override {}

};

#endif // _KARBONTOOLSPLUGIN_H_
