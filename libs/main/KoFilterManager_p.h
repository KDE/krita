/* This file is part of the KDE project
   Copyright (C) 2003 Clarence Dang <dang@kde.org>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License Version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License Version 2 for more details.

   You should have received a copy of the GNU Library General Public License
   Version 2 along with this library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef __koFilterManager_p_h__
#define __koFilterManager_p_h__

#include <QString>
#include <QStringList>

#include <KUrl>
#include <KDialog>

class QListWidget;

class KoFilterManager::Private
{
public:
    bool batch;
    QByteArray importMimeType;
};

class KoFilterChooser : public KDialog
{
public:
    KoFilterChooser(QWidget *parent, const QStringList &mimeTypes,
                    const QString &nativeFormat = QString(), const KUrl &url = KUrl());
    ~KoFilterChooser();

    QString filterSelected();

private:
    QStringList m_mimeTypes;
    QListWidget *m_filterList;
};

#endif
