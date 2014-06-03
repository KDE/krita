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

#ifndef GOOGLEDOCUMENT_H
#define GOOGLEDOCUMENT_H

#include <QObject>

class GoogleDocument : public QObject
{
    Q_OBJECT

public:
    GoogleDocument();

    void setEtag(const QString & etag);
    QString etag () const;

    void setId (const QString & id);
    QString id () const;

    void setTitle (const QString & title);
    QString title () const;

    void setAuthor (const QString & author);
    QString author () const;

    void setDocumentType (const QString & docType);
    QString documentType () const;

    void setDocumentUrl (const QString & url);
    QString documentUrl () const;

private:
    QString m_etag;
    QString m_id;
    QString m_title;
    QString m_author;
    QString m_docType;
    QString m_documetUrl;
};

#endif // GOOGLEDOCUMENT_H
