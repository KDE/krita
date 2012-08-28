/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2006 Fredrik Edemar <f_edemar@linux.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KODOCUMENTADAPTOR_H
#define KODOCUMENTADAPTOR_H

#include <QMap>

#include <QObject>
#include <QtDBus/qdbusabstractadaptor.h>
#include <QList>
#include "komain_export.h"
class KoPart;

/**
 * DBUS interface for any Calligra document
 * Use KoApplicationIface to get hold of an existing document's interface,
 * or to create a document.
 */
class KOMAIN_EXPORT KoPartAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.calligra.document")
public:

    explicit KoPartAdaptor(KoPart* doc);
    ~KoPartAdaptor();

public slots: // METHODS
    /**
     * Returns the URL for this document (empty, real URL, or internal one)
     */
    Q_SCRIPTABLE QString url();

    /**
     * Opens a document stored in @p url
     * Warning: this is asynchronous. The document might not be loaded yet when
     * this call returns. See isLoading.
     */
    Q_SCRIPTABLE void openUrl(const QString & url);

    /**
     * @return TRUE is the document is still loading
     */
    Q_SCRIPTABLE bool isLoading();

    /**
     * @return TRUE is the document has been modified
     */
    Q_SCRIPTABLE bool isModified();

    /**
     * @return the number of views this document is displayed in
     */
    Q_SCRIPTABLE int viewCount();

    /**
     * @return a representing the view with index @p idx
     */
    Q_SCRIPTABLE QString view(int idx);

    /**
     * @return list of actions
     */
    Q_SCRIPTABLE QStringList actions();

    /**
     * Saves the document under its existing filename
     */
    Q_SCRIPTABLE void save();

    /**
     * Saves the document under a new name
     */
    Q_SCRIPTABLE void saveAs(const QString & url);

    Q_SCRIPTABLE void setOutputMimeType(const QByteArray & mimetype);

    Q_SCRIPTABLE QString documentInfoAuthorName() const;
    Q_SCRIPTABLE QString documentInfoEmail() const;
    Q_SCRIPTABLE QString documentInfoCompanyName() const;
    Q_SCRIPTABLE QString documentInfoTitle() const;
    Q_SCRIPTABLE QString documentInfoAbstract() const;
    Q_SCRIPTABLE QString documentInfoKeywords() const;
    Q_SCRIPTABLE QString documentInfoSubject() const;
    Q_SCRIPTABLE QString documentInfoTelephone() const;
    Q_SCRIPTABLE QString documentInfoTelephoneWork() const;
    Q_SCRIPTABLE QString documentInfoTelephoneHome() const;
    Q_SCRIPTABLE QString documentInfoFax() const;
    Q_SCRIPTABLE QString documentInfoCountry() const;
    Q_SCRIPTABLE QString documentInfoPostalCode() const;
    Q_SCRIPTABLE QString documentInfoCity() const;
    Q_SCRIPTABLE QString documentInfoStreet() const;
    Q_SCRIPTABLE QString documentInfoInitial() const;
    Q_SCRIPTABLE QString documentInfoAuthorPostion() const;
    Q_SCRIPTABLE void setDocumentInfoAuthorName(const QString & text);
    Q_SCRIPTABLE void setDocumentInfoEmail(const QString &text);
    Q_SCRIPTABLE void setDocumentInfoCompanyName(const QString &text);
    Q_SCRIPTABLE void setDocumentInfoTelephone(const QString &text);
    Q_SCRIPTABLE void setDocumentInfoTelephoneWork(const QString &text);
    Q_SCRIPTABLE void setDocumentInfoTelephoneHome(const QString &text);
    Q_SCRIPTABLE void setDocumentInfoFax(const QString &text);
    Q_SCRIPTABLE void setDocumentInfoCountry(const QString &text);
    Q_SCRIPTABLE void setDocumentInfoTitle(const QString & text);
    Q_SCRIPTABLE void setDocumentInfoPostalCode(const QString &text);
    Q_SCRIPTABLE void setDocumentInfoCity(const QString & text);
    Q_SCRIPTABLE void setDocumentInfoStreet(const QString &text);
    Q_SCRIPTABLE void setDocumentInfoAbstract(const QString &text);
    Q_SCRIPTABLE void setDocumentInfoInitial(const QString & text);
    Q_SCRIPTABLE void setDocumentInfoKeywords(const QString & text);
    Q_SCRIPTABLE void setDocumentInfoSubject(const QString & text);
    Q_SCRIPTABLE void setDocumentInfoAuthorPosition(const QString & text);

public:

protected:
    KoPart* m_pDoc;
};

#endif

