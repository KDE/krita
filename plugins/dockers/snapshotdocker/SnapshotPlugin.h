/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Matus Talcik <matus.talcik@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SNAPSHOT_PLUGIN_H_
#define SNAPSHOT_PLUGIN_H_

#include <QObject>
#include <QVariant>

class SnapshotPlugin : public QObject
{
    Q_OBJECT
public:
    SnapshotPlugin(QObject *parent, const QVariantList &);
    ~SnapshotPlugin() override;
};

#endif
