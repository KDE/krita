/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef __rdf_KoRdfFoaF_h__
#define __rdf_KoRdfFoaF_h__

#include "KoRdfSemanticItem.h"

// contacts
#ifdef KDEPIMLIBS_FOUND
namespace KABC {
    class Addressee;
}
class KJob;
#endif

#include "ui_KoRdfFoaFEditWidget.h"

/**
 * @short Contact information from the FOAF vocabulary.
 * @author Ben Martin <ben.martin@kogmbh.com>
 */
class KoRdfFoaF : public KoRdfSemanticItem
{
    Q_OBJECT
public:

    explicit KoRdfFoaF(QObject *parent, const KoDocumentRdf *m_rdf = 0);
    KoRdfFoaF(QObject *parent, const KoDocumentRdf *m_rdf, Soprano::QueryResultIterator &it);
    virtual ~KoRdfFoaF();

    // inherited and reimplemented...

    /**
     * Export to a VCard format file
     * Prompt for a filename if none is given
     */
    void exportToFile(const QString& fileName = QString()) const;
    /**
     * Import from VCard data contained in ba.
     */
    virtual void importFromData(const QByteArray &ba, const KoDocumentRdf *rdf = 0, KoCanvasBase *host = 0);
    virtual QWidget *createEditor(QWidget *parent);
    virtual void updateFromEditorData();
    virtual KoRdfSemanticTreeWidgetItem *createQTreeWidgetItem(QTreeWidgetItem *parent = 0);
    virtual Soprano::Node linkingSubject() const;
    virtual void setupStylesheetReplacementMapping(QMap<QString, QString> &m);
    virtual void exportToMime(QMimeData *md) const;
    virtual QList<hKoSemanticStylesheet> stylesheets() const;
    virtual QString className() const;

    /**
     * Export the contact to your current KDE addressbook.
     */
    void saveToKABC();

    // accessor methods...

    virtual QString name() const;

private:
    // foaf Rdf template,
    // s == m_uri
    // s -> <uri:gollum>; p -> <http://xmlns.com/foaf/0.1/name>; o -> "Gollum"
    // s-> <uri:gollum>; p -> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type>; o -> <http://xmlns.com/foaf/0.1/Person>
    QString m_uri;   // This is the subject in Rdf
    QString m_name;
    QString m_nick;
    QString m_homePage;
    QString m_imageUrl;
    QString m_phone;
    Ui::KoRdfFoaFEditWidget editWidget;

#ifdef KDEPIMLIBS_FOUND
    KABC::Addressee toKABC() const;
    void fromKABC(KABC::Addressee addr);
private Q_SLOTS:
    void onCreateJobFinished(KJob *job);
#endif

};

typedef QExplicitlySharedDataPointer<KoRdfFoaF> hKoRdfFoaF;

#endif
