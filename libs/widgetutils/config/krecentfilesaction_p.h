/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KRECENTFILESACTION_P_H
#define KRECENTFILESACTION_P_H

#include "krecentfilesaction.h"

class KRecentFilesActionPrivate
{
    Q_DECLARE_PUBLIC(KRecentFilesAction)

public:
    KRecentFilesActionPrivate(KRecentFilesAction *parent)
        : q_ptr(parent)
    {
        m_visibleItemsCount = 10;
        m_noEntriesAction = 0;
        clearSeparator = 0;
        clearAction = 0;
        m_recentFilesModel = 0;
        m_fileIconsPopulated = false;
    }

    virtual ~KRecentFilesActionPrivate()
    {
    }

    void init();

    void _k_urlSelected(QAction *);

    void updateIcon(const QStandardItem *item);

    int m_visibleItemsCount;
    QMap<QAction *, QUrl> m_urls;
    QAction *m_noEntriesAction;
    QAction *clearSeparator;
    QAction *clearAction;
    const QStandardItemModel *m_recentFilesModel;
    bool m_fileIconsPopulated;

    KRecentFilesAction *q_ptr;
};

#endif // KRECENTFILESACTION_P_H
