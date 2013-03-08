/*
 *  Copyright (c) 2010 Mani Chandrasekar <maninc@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef GOOGLEDOCUMENTLIST_H
#define GOOGLEDOCUMENTLIST_H

#include <QObject>
#include <QList>
#include <QStandardItemModel>

class GoogleDocument;

class GoogleDocumentList : public QObject
{
    Q_OBJECT

public:
    GoogleDocumentList();

    void setEtag(const QString &etag);
    QString etag ();

    void setEmail (const QString &email);
    QString email ();

    void setTitle (const QString &title);
    QString title ();

    void setAuthor (const QString &author);
    QString author ();

    void append(GoogleDocument *entry);
    int documentsCount();

    QList<GoogleDocument *> entries ();

    QStandardItemModel * documentModel() {  return docModel; }

private:
    QString m_author;
    QString m_email;
    QString m_title;
    QString m_etag;

    QList <GoogleDocument *>m_entries;

    QStandardItemModel *docModel;
};

#endif // GOOGLEDOCUMENTLIST_H
