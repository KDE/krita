/*
 *  SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DBEXPLORER_H
#define DBEXPLORER_H

#include <QVariant>
#include <KisActionPlugin.h>

class KUndo2MagicString;

class DbExplorer : public KisActionPlugin
{
    Q_OBJECT
public:
    DbExplorer(QObject *parent, const QVariantList &);
    ~DbExplorer() override;

public Q_SLOTS:

    void slotDbExplorer();

};

#endif // DBEXPLORER_H
