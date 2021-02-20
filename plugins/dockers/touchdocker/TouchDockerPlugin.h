/*
 *  SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TOUCHDOCKERPLUGIN_H
#define TOUCHDOCKERPLUGIN_H

#include <QObject>
#include <QVariant>

class TouchDockerPlugin : public QObject
{
    Q_OBJECT
public:
    TouchDockerPlugin(QObject *parent, const QVariantList &);
    ~TouchDockerPlugin() override;
};

#endif
