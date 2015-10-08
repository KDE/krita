/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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
        m_maxItems = 10;
        m_noEntriesAction = 0;
        clearSeparator = 0;
        clearAction = 0;
    }

    virtual ~KRecentFilesActionPrivate()
    {
    }

    void init();

    void _k_urlSelected(QAction *);

    int m_maxItems;
    QMap<QAction *, QString> m_shortNames;
    QMap<QAction *, QUrl> m_urls;
    QAction *m_noEntriesAction;
    QAction *clearSeparator;
    QAction *clearAction;

    KRecentFilesAction *q_ptr;
};

#endif // KRECENTFILESACTION_P_H
