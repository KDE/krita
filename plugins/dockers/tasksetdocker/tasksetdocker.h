/*
 *  SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TASKSETDOCKER_H
#define TASKSETDOCKER_H

#include <QObject>
#include <QVariant>


/**
 * Docker showing the channels of the current layer
 */
class TasksetDockerPlugin : public QObject
{
    Q_OBJECT
public:
    TasksetDockerPlugin(QObject *parent, const QVariantList &);
    ~TasksetDockerPlugin() override;
};

#endif
