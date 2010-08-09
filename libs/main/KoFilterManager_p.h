/* This file is part of the KDE project
   Copyright (C) 2003 Clarence Dang <dang@kde.org>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef __koFilterManager_p_h__
#define __koFilterManager_p_h__

#include <QString>
#include <QStringList>

#include <KUrl>
#include <KDialog>

class QListWidget;
class KoProgressUpdater;

class KoFilterManager::Private
{
public:
    bool batch;
    QByteArray importMimeType;
    KoProgressUpdater * const progressUpdater;

    Private(KoProgressUpdater *progressUpdater_ = 0)
        :progressUpdater(progressUpdater_)
    {
    }

};

class KoFilterChooser : public KDialog
{
    Q_OBJECT

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
