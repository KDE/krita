/*
 *  SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef _LOG_DOCKER_H_
#define _LOG_DOCKER_H_

#include <QObject>
#include <QVariant>

class LogDockerPlugin : public QObject
{
    Q_OBJECT
public:
    LogDockerPlugin(QObject *parent, const QVariantList &);
    ~LogDockerPlugin() override;
};

#endif
