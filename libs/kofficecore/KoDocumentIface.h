/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#include <qmap.h>

#include <dcopobject.h>
#include <q3valuelist.h>
//Added by qt3to4:
#include <Q3CString>
#include <dcopref.h>
#include <koffice_export.h>
class KoDocument;
class KDCOPActionProxy;

/**
 * DCOP interface for any KOffice document
 * Use KoApplicationIface to get hold of an existing document's interface,
 * or to create a document.
 *
 * Note: KOffice Applications may (and should) reimplement KoDocument::dcopObject()
 * In this case, don't look here... (unless the DCOP interface for the document
 * inherits KoDocumentIface, which is a good thing to do)
 */
class KOFFICECORE_EXPORT KoDocumentIface : public DCOPObject
{
    K_DCOP
public:

    KoDocumentIface( KoDocument * doc, const char * name = 0 );
    ~KoDocumentIface();

    /**
     * Generate a name for this interface. Automatically used if name=0 is
     * passed to the constructor
     */
    static Q3CString newIfaceName();

k_dcop:
    /**
     * Returns the URL for this document (empty, real URL, or internal one)
     */
    QString url();

    /**
     * Opens a document stored in @p url
     * Warning: this is asynchronous. The document might not be loaded yet when
     * this call returns. See isLoading.
     */
    void openURL( QString url );

    /**
     * @return TRUE is the document is still loading
     */
    bool isLoading();

    /**
     * @return TRUE is the document has been modified
     */
    bool isModified();

    /**
     * @return the number of views this document is displayed in
     */
    int viewCount();

    /**
     * @return a DCOP reference (DCOPRef) to the view with index @p idx
     */
    DCOPRef view( int idx );

    /**
     * DCOP-action proxy
     */
    DCOPRef action( const Q3CString &name );
    /**
     * @return list of actions
     */
    DCOPCStringList actions();
    /**
     * @return a map of (action name, DCOP reference)
     */
    QMap<DCOPCString,DCOPRef> actionMap();

    /**
     * Saves the document under its existing filename
     */
    void save();

    /**
     * Saves the document under a new name
     */
    void saveAs( const QString & url );

    void setOutputMimeType( const Q3CString & mimetype );

    QString documentInfoAuthorName() const;
    QString documentInfoEmail() const;
    QString documentInfoCompanyName() const;
    QString documentInfoTitle() const;
    QString documentInfoAbstract() const;
    QString documentInfoKeywords() const;
    QString documentInfoSubject() const;
    QString documentInfoTelephone() const;
    QString documentInfoTelephoneWork() const;
    QString documentInfoTelephoneHome() const;
    QString documentInfoFax() const;
    QString documentInfoCountry() const;
    QString documentInfoPostalCode() const;
    QString documentInfoCity() const;
    QString documentInfoStreet() const;
    QString documentInfoInitial() const;
    QString documentInfoAuthorPostion() const;
    void setDocumentInfoAuthorName(const QString & text);
    void setDocumentInfoEmail(const QString &text);
    void setDocumentInfoCompanyName(const QString &text);
    void setDocumentInfoTelephone(const QString &text);
    void setDocumentInfoTelephoneWork(const QString &text);
    void setDocumentInfoTelephoneHome(const QString &text);
    void setDocumentInfoFax(const QString &text);
    void setDocumentInfoCountry(const QString &text);
    void setDocumentInfoTitle(const QString & text);
    void setDocumentInfoPostalCode(const QString &text);
    void setDocumentInfoCity(const QString & text);
    void setDocumentInfoStreet(const QString &text);
    void setDocumentInfoAbstract(const QString &text);
    void setDocumentInfoInitial(const QString & text);
    void setDocumentInfoKeywords(const QString & text );
    void setDocumentInfoSubject(const QString & text);
    void setDocumentInfoAuthorPosition(const QString & text);

public:
    virtual DCOPCStringList functionsDynamic();
    virtual bool processDynamic( const DCOPCString &fun, const QByteArray &data,
                                 DCOPCString& replyType, QByteArray &replyData );

protected:
    KoDocument * m_pDoc;
    KDCOPActionProxy *m_actionProxy;
};

#endif

