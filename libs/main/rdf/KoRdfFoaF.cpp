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

#include "KoRdfFoaF.h"
#include "KoDocumentRdf.h"
#include "KoDocumentRdf_p.h"
#include "KoRdfSemanticItem_p.h"
#include "KoTextRdfCore.h"

#include <QUuid>
#include <QTemporaryFile>
#include <kdebug.h>
#include <kfiledialog.h>

// contacts
#ifdef KDEPIMLIBS_FOUND
#include <kabc/addressee.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addressbook.h>
#include <kabc/phonenumber.h>
#include <kabc/vcardconverter.h>
#endif

#include "ui_KoRdfFoaFEditWidget.h"

using namespace Soprano;

class KoRdfFoaFPrivate : public KoRdfSemanticItemPrivate
{
    // foaf Rdf template,
    // s == m_uri
    // s -> <uri:gollum>; p -> <http://xmlns.com/foaf/0.1/name>; o -> "Gollum"
    // s-> <uri:gollum>; p -> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type>; o -> <http://xmlns.com/foaf/0.1/Person>
public:
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
#endif

    KoRdfFoaFPrivate(const KoDocumentRdf *rdf)
        : KoRdfSemanticItemPrivate(rdf)
        {}

    KoRdfFoaFPrivate(const KoDocumentRdf *rdf, Soprano::QueryResultIterator &it)
        : KoRdfSemanticItemPrivate(rdf, it)
        {}
};

KoRdfFoaF::KoRdfFoaF(QObject *parent, const KoDocumentRdf *m_rdf)
    : KoRdfSemanticItem(*new KoRdfFoaFPrivate(m_rdf), parent)
{
}

KoRdfFoaF::KoRdfFoaF(QObject *parent, const KoDocumentRdf *m_rdf, Soprano::QueryResultIterator &it)
    : KoRdfSemanticItem(*new KoRdfFoaFPrivate(m_rdf,it), parent)
{
    Q_D (KoRdfFoaF);
    d->m_uri      = it.binding("person").toString();
    d->m_name     = it.binding("name").toString();
    d->m_nick     = KoTextRdfCore::optionalBindingAsString(it, "nick");
    d->m_homePage = KoTextRdfCore::optionalBindingAsString(it, "homepage");
    d->m_imageUrl = KoTextRdfCore::optionalBindingAsString(it, "img");
    d->m_phone    = KoTextRdfCore::optionalBindingAsString(it, "phone");
    kDebug(30015) << "+++xmlid:" << it.binding("xmlid").toString();
}

KoRdfFoaF::~KoRdfFoaF()
{
    kDebug(30015) << "~KoRdfFoaF() this:" << this << " name:" << name();
}


QString KoRdfFoaF::name() const
{
    Q_D (const KoRdfFoaF);
    return d->m_name;
}

QWidget* KoRdfFoaF::createEditor(QWidget * parent)
{
    Q_D (KoRdfFoaF);
    QWidget *ret = new QWidget(parent);
    d->editWidget.setupUi(ret);
    d->editWidget.name->setText(d->m_name);
    d->editWidget.nick->setText(d->m_nick);
    d->editWidget.url->setText(d->m_homePage);
    d->editWidget.phone->setText(d->m_phone);
    return ret;
}

void KoRdfFoaF::updateFromEditorData()
{
    Q_D (KoRdfFoaF);
    if (d->m_uri.size() <= 0) {
        QUuid u = QUuid::createUuid();
        d->m_uri = u.toString();
        kDebug(30015) << "no uuid, created one:" << u.toString();
    }
    QString predBase = "http://xmlns.com/foaf/0.1/";
    kDebug(30015) << "name:" << d->m_name << " newV:" << d->editWidget.name->text();
    setRdfType(predBase + "Person");
    updateTriple(d->m_name, d->editWidget.name->text(), predBase + "name");
    updateTriple(d->m_nick, d->editWidget.nick->text(), predBase + "nick");
    updateTriple(d->m_homePage, d->editWidget.url->text(), predBase + "homepage");
    updateTriple(d->m_phone, d->editWidget.phone->text(), predBase + "phone");
    if (documentRdf()) {
        const_cast<KoDocumentRdf*>(documentRdf())->emitSemanticObjectUpdated(this);
    }
}

KoRdfSemanticTreeWidgetItem *KoRdfFoaF::createQTreeWidgetItem(QTreeWidgetItem *parent)
{
    kDebug(30015) << "format(), default stylesheet:" << defaultStylesheet()->name();
    KoRdfFoaFTreeWidgetItem *item = new KoRdfFoaFTreeWidgetItem(parent, this);
    return item;
}

Soprano::Node KoRdfFoaF::linkingSubject() const
{
    Q_D (const KoRdfFoaF);
    return Node::createResourceNode(d->m_uri);
}

void KoRdfFoaF::setupStylesheetReplacementMapping(QMap<QString, QString> &m)
{
    Q_D (KoRdfFoaF);
    m["%URI%"] = d->m_uri;
    m["%NICK%"] = d->m_nick;
    m["%HOMEPAGE%"] = d->m_homePage;
    m["%PHONE%"] = d->m_phone;
}

void KoRdfFoaF::exportToMime(QMimeData *md) const
{
    Q_D (const KoRdfFoaF);
    QTemporaryFile file;
    if (file.open()) {
        //QString mimeType = "text/directory"; // text/x-vcard";
        exportToFile(file.fileName());
        QByteArray ba = KoTextRdfCore::fileToByteArray(file.fileName());
        md->setData("text/directory", ba);
        md->setData("text/x-vcard", ba);
        kDebug(30015) << "ba.sz:" << ba.size();
        kDebug(30015) << "ba:" << ba;
    }
    QString data;
    QTextStream oss(&data);
    oss << name() << ", " << d->m_phone << flush;
    md->setText(data);
}

QList<KoSemanticStylesheet*> KoRdfFoaF::stylesheets() const
{
    QList<KoSemanticStylesheet*> stylesheets;
    stylesheets.append(
        new KoSemanticStylesheet("143c1ba3-d7bb-440b-8528-7f07d2eff5f2", "name", "%NAME%"));
    stylesheets.append(
        new KoSemanticStylesheet("2fad34d1-42a0-4b10-b17e-a87db5208f6d", "nick", "%NICK%"));
    stylesheets.append(
        new KoSemanticStylesheet("0dd5878d-95c5-47e5-a777-63ec36da3b9a", "name, phone", "%NAME%, %PHONE%"));
    stylesheets.append(
        new KoSemanticStylesheet("9cbeb4a6-34c5-49b2-b3ef-b94277db0c59", "nick, phone", "%NICK%, %PHONE%"));
    stylesheets.append(
        new KoSemanticStylesheet("47025a4a-5da5-4a32-8d89-14c03658631d", "name, (homepage), phone", "%NAME%, (%HOMEPAGE%), %PHONE%"));
    return stylesheets;
}

QString KoRdfFoaF::className() const
{
    return "Contact";
}

#ifdef KDEPIMLIBS_FOUND
KABC::Addressee KoRdfFoaFPrivate::toKABC() const
{
    KABC::Addressee addr;
    addr.setNameFromString(m_name);
    addr.setNickName(m_nick);
    KABC::PhoneNumber ph(m_phone, KABC::PhoneNumber::Work);
    addr.insertPhoneNumber(ph);
    return addr;
}

void KoRdfFoaFPrivate::fromKABC(KABC::Addressee addr)
{
    m_name = addr.realName();
    m_nick = addr.nickName();
    KABC::PhoneNumber ph = addr.phoneNumber(KABC::PhoneNumber::Work);
    m_phone = ph.number();
    m_homePage = addr.url().url();
}
#endif

void KoRdfFoaF::saveToKABC()
{
    Q_D (KoRdfFoaF);
    kDebug(30015) << "saving name:" << d->m_name;
#ifdef KDEPIMLIBS_FOUND
    KABC::StdAddressBook *ab = static_cast<KABC::StdAddressBook*>
                               (KABC::StdAddressBook::self());
    if (!ab) {
        return;
    }
    KABC::Ticket *ticket = ab->requestSaveTicket();
    KABC::Addressee addr = d->toKABC();
    ab->insertAddressee(addr);
    KABC::AddressBook* pab = ab;
    pab->save(ticket);
#endif
}

void KoRdfFoaF::exportToFile(const QString &fileNameConst) const
{
    Q_D (const KoRdfFoaF);
    QString fileName = fileNameConst;

#ifdef KDEPIMLIBS_FOUND
    if (!fileName.size()) {
        fileName = KFileDialog::getSaveFileName(
                       KUrl("kfiledialog:///ExportDialog"),
                       "*.vcf|vCard files",
                       0,
                       "Export to selected vCard file");

        if (!fileName.size()) {
            kDebug(30015) << "no filename given, cancel export..";
            return;
        }
    }
    KABC::Addressee addr = d->toKABC();
    KABC::VCardConverter converter;
    QByteArray ba = converter.createVCard(addr);
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    file.write(ba);
    file.close();
    kDebug(30015) << "wrote " << ba.size() << " bytes to export file:" << fileName;
#else
    kDebug(30015) << "KDEPIM support not built!";
#endif
}

void KoRdfFoaF::importFromData(const QByteArray &ba, KoDocumentRdf *_rdf, KoCanvasBase *host)
{
    Q_D (KoRdfFoaF);
#ifdef KDEPIMLIBS_FOUND
    kDebug(30015) << "data.sz:" << ba.size();
    kDebug(30015) << "_rdf:" << _rdf;
    if (_rdf) {
        d->m_rdf = _rdf;
    }
    KABC::VCardConverter converter;
    KABC::Addressee addr = converter.parseVCard(ba);
    d->fromKABC(addr);
    kDebug(30015) << "adding name:" << d->m_name;
    kDebug(30015) << "uri:" << d->m_uri;
    importFromDataComplete(ba, documentRdf(), host);
#else
    kDebug(30015) << "KDEPIM support not built!";
#endif
}
