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
#include "KoRdfSemanticItem_p.h"
#include "KoTextRdfCore.h"
#include "KoRdfFoaFTreeWidgetItem.h"
#include <QUuid>
#include <QTemporaryFile>
#include <kdebug.h>
#include <kfiledialog.h>

using namespace Soprano;


KoRdfFoaF::KoRdfFoaF(QObject *parent, const KoDocumentRdf *m_rdf)
    : KoRdfSemanticItem(m_rdf, parent)
{
}

KoRdfFoaF::KoRdfFoaF(QObject *parent, const KoDocumentRdf *m_rdf, Soprano::QueryResultIterator &it)
    : KoRdfSemanticItem(m_rdf, it, parent)
{
    m_uri      = it.binding("person").toString();
    m_name     = it.binding("name").toString();
    m_nick     = KoTextRdfCore::optionalBindingAsString(it, "nick");
    m_homePage = KoTextRdfCore::optionalBindingAsString(it, "homepage");
    m_imageUrl = KoTextRdfCore::optionalBindingAsString(it, "img");
    m_phone    = KoTextRdfCore::optionalBindingAsString(it, "phone");
    kDebug(30015) << "+++xmlid:" << it.binding("xmlid").toString();
}

KoRdfFoaF::~KoRdfFoaF()
{
    kDebug(30015) << "~KoRdfFoaF() this:" << this << " name:" << name();
}


QString KoRdfFoaF::name() const
{
    return m_name;
}

QWidget* KoRdfFoaF::createEditor(QWidget * parent)
{
    QWidget *ret = new QWidget(parent);
    editWidget.setupUi(ret);
    editWidget.name->setText(m_name);
    editWidget.nick->setText(m_nick);
    editWidget.url->setText(m_homePage);
    editWidget.phone->setText(m_phone);
    return ret;
}

void KoRdfFoaF::updateFromEditorData()
{
    if (m_uri.size() <= 0) {
        QUuid u = QUuid::createUuid();
        m_uri = u.toString();
        kDebug(30015) << "no uuid, created one:" << u.toString();
    }
    QString predBase = "http://xmlns.com/foaf/0.1/";
    kDebug(30015) << "name:" << m_name << " newV:" << editWidget.name->text();
    setRdfType(predBase + "Person");
    updateTriple(m_name, editWidget.name->text(), predBase + "name");
    updateTriple(m_nick, editWidget.nick->text(), predBase + "nick");
    updateTriple(m_homePage, editWidget.url->text(), predBase + "homepage");
    updateTriple(m_phone, editWidget.phone->text(), predBase + "phone");
    if (documentRdf()) {
        const_cast<KoDocumentRdf*>(documentRdf())->emitSemanticObjectUpdated(hKoRdfSemanticItem(this));
    }
}

KoRdfSemanticTreeWidgetItem *KoRdfFoaF::createQTreeWidgetItem(QTreeWidgetItem *parent)
{
    kDebug(30015) << "format(), default stylesheet:" << defaultStylesheet()->name();
    KoRdfFoaFTreeWidgetItem *item = new KoRdfFoaFTreeWidgetItem(parent, hKoRdfSemanticItem(this));
    return item;
}

Soprano::Node KoRdfFoaF::linkingSubject() const
{
    return Node::createResourceNode(m_uri);
}

void KoRdfFoaF::setupStylesheetReplacementMapping(QMap<QString, QString> &m)
{
    m["%URI%"] = m_uri;
    m["%NICK%"] = m_nick;
    m["%HOMEPAGE%"] = m_homePage;
    m["%PHONE%"] = m_phone;
}

void KoRdfFoaF::exportToMime(QMimeData *md) const
{
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
    oss << name() << ", " << m_phone << flush;
    md->setText(data);
}

QList<hKoSemanticStylesheet> KoRdfFoaF::stylesheets() const
{
    QList<hKoSemanticStylesheet> stylesheets;
    stylesheets.append(
        hKoSemanticStylesheet(new KoSemanticStylesheet("143c1ba3-d7bb-440b-8528-7f07d2eff5f2", "name", "%NAME%")));
    stylesheets.append(
        hKoSemanticStylesheet(new KoSemanticStylesheet("2fad34d1-42a0-4b10-b17e-a87db5208f6d", "nick", "%NICK%")));
    stylesheets.append(
        hKoSemanticStylesheet(new KoSemanticStylesheet("0dd5878d-95c5-47e5-a777-63ec36da3b9a", "name, phone", "%NAME%, %PHONE%")));
    stylesheets.append(
        hKoSemanticStylesheet(new KoSemanticStylesheet("9cbeb4a6-34c5-49b2-b3ef-b94277db0c59", "nick, phone", "%NICK%, %PHONE%")));
    stylesheets.append(
        hKoSemanticStylesheet(new KoSemanticStylesheet("47025a4a-5da5-4a32-8d89-14c03658631d", "name, (homepage), phone", "%NAME%, (%HOMEPAGE%), %PHONE%")));
    return stylesheets;
}

QString KoRdfFoaF::className() const
{
    return "Contact";
}

#ifdef KDEPIMLIBS_FOUND
KABC::Addressee KoRdfFoaF::toKABC() const
{
    KABC::Addressee addr;
    addr.setNameFromString(m_name);
    addr.setNickName(m_nick);
    KABC::PhoneNumber ph(m_phone, KABC::PhoneNumber::Work);
    addr.insertPhoneNumber(ph);
    return addr;
}

void KoRdfFoaF::fromKABC(KABC::Addressee addr)
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
    kDebug(30015) << "saving name:" << m_name;
#ifdef KDEPIMLIBS_FOUND
    KABC::StdAddressBook *ab = static_cast<KABC::StdAddressBook*>
                               (KABC::StdAddressBook::self());
    if (!ab) {
        return;
    }
    KABC::Ticket *ticket = ab->requestSaveTicket();
    KABC::Addressee addr = toKABC();
    ab->insertAddressee(addr);
    KABC::AddressBook* pab = ab;
    pab->save(ticket);
#endif
}

void KoRdfFoaF::exportToFile(const QString &fileNameConst) const
{
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
    KABC::Addressee addr = toKABC();
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
#ifdef KDEPIMLIBS_FOUND
    kDebug(30015) << "data.sz:" << ba.size();
    kDebug(30015) << "_rdf:" << _rdf;
    if (_rdf) {
        m_rdf = _rdf;
    }
    KABC::VCardConverter converter;
    KABC::Addressee addr = converter.parseVCard(ba);
    fromKABC(addr);
    kDebug(30015) << "adding name:" << m_name;
    kDebug(30015) << "uri:" << m_uri;
    importFromDataComplete(ba, documentRdf(), host);
#else
    kDebug(30015) << "KDEPIM support not built!";
#endif
}
