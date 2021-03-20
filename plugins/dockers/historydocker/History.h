/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Matus Talcik <matus.talcik@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _HISTORY_H_
#define _HISTORY_H_

#include <QObject>
#include <QVariant>

class HistoryPlugin : public QObject
{
    Q_OBJECT
public:
    HistoryPlugin(QObject *parent, const QVariantList &);
    ~HistoryPlugin() override;
};

#endif
