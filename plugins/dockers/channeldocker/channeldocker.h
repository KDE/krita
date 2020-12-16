/*
 *  SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef CHANNELDOCKER_H
#define CHANNELDOCKER_H

#include <QObject>
#include <QVariant>


/**
 * Docker showing the channels of the current layer
 */
class ChannelDockerPlugin : public QObject
{
    Q_OBJECT
public:
    ChannelDockerPlugin(QObject *parent, const QVariantList &);
    ~ChannelDockerPlugin() override;
};

#endif
