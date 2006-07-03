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

#ifndef __KoDocumentIface_h__
#define __KoDocumentIface_h__

#include <QMap>

#include <QtCore/QObject>
#include <QtDBus/qdbusabstractadaptor.h>
#include <q3valuelist.h>
#include <koffice_export.h>
class KoDocument;

/**
 * DBUS interface for any KOffice document
 * Use KoApplicationIface to get hold of an existing document's interface,
 * or to create a document.
 *
 * Note: KOffice Applications may (and should) reimplement KoDocument::dcopObject()
 * In this case, don't look here... (unless the DCOP interface for the document
 * inherits KoDocumentAdaptor, which is a good thing to do)
 */
class KOFFICECORE_EXPORT KoDocumentAdaptor : public QDBusAbstractAdaptor
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.koffice.document")
public:

    KoDocumentAdaptor( KoDocument * doc);
    ~KoDocumentAdaptor();

public Q_SLOTS: // METHODS
    /**
     * Returns the URL for this document (empty, real URL, or internal one)
     */
    Q_SCRIPTABLE QString url();

    /**
     * Opens a document stored in @p url
     * Warning: this is asynchronous. The document might not be loaded yet when
     * this call returns. See isLoading.
     */
    Q_SCRIPTABLE void openURL( QString url );

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
     * @return a DCOP reference (DCOPRef) to the view with index @p idx
     */
    Q_SCRIPTABLE QString view( int idx );

    /**
     * DCOP-action proxy
     */
//     DCOPRef action( const DCOPCString &name );
    /**
     * @return list of actions
     */
    Q_SCRIPTABLE QStringList actions();
    /**
     * @return a map of (action name, DCOP reference)
     */
//     QMap<DCOPCString,DCOPRef> actionMap();

    /**
     * Saves the document under its existing filename
     */
    Q_SCRIPTABLE void save();

    /**
     * Saves the document under a new name
     */
    Q_SCRIPTABLE void saveAs( const QString & url );

    Q_SCRIPTABLE void setOutputMimeType( const QByteArray & mimetype );

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
    Q_SCRIPTABLE void setDocumentInfoKeywords(const QString & text );
    Q_SCRIPTABLE void setDocumentInfoSubject(const QString & text);
    Q_SCRIPTABLE void setDocumentInfoAuthorPosition(const QString & text);

public:
//     virtual DCOPCStringList functionsDynamic();
//     virtual bool processDynamic( const DCOPCString &fun, const QByteArray &data,
//                                  DCOPCString& replyType, QByteArray &replyData );

protected:
    KoDocument * m_pDoc;
//     KDCOPActionProxy *m_actionProxy;
};

#endif

