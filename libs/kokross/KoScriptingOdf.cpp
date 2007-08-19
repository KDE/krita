/***************************************************************************
 * KoScriptingOdf.cpp
 * This file is part of the KDE project
 * copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "KoScriptingOdf.h"

#include <QPointer>
#include <QIODevice>
#include <QBuffer>
#include <QRegExp>
#include <KoStore.h>
#include <KoOasisStore.h>
#include <KoDocumentAdaptor.h>

/************************************************************************************************
 * KoScriptingOdfReader
 */

/// \internal d-pointer class.
class KoScriptingOdfReader::Private
{
    public:
        KoXmlDocument doc;
        KoXmlElement currentElement;
        int level;
        QString filter;
        QRegExp filterRegExp;
        explicit Private(const KoXmlDocument& doc) : doc(doc), level(0), filterRegExp(false) {}
};

KoScriptingOdfReader::KoScriptingOdfReader(QObject* parent, const KoXmlDocument& doc) : QObject(parent), d(new Private(doc)) {
}

KoScriptingOdfReader::~KoScriptingOdfReader() {
    delete d;
}

void KoScriptingOdfReader::start() {
    KoXmlElement elem = d->doc.documentElement();
    handleElement(elem);
    setCurrentElement(KoXmlElement());
    setLevel(0);
}

QString KoScriptingOdfReader::nameFilter() const { return d->filter; }
void KoScriptingOdfReader::setNameFilter(const QString& name, bool regularExpression) const {
    d->filter = name.isEmpty() ? QString() : name;
    d->filterRegExp = regularExpression ? QRegExp(name, Qt::CaseInsensitive) : QRegExp();
}

KoXmlDocument KoScriptingOdfReader::doc() const { return d->doc; }
KoXmlElement KoScriptingOdfReader::currentElement() const { return d->currentElement; }

QString KoScriptingOdfReader::name() const { return d->currentElement.tagName(); /*.nodeName();*/ }
QString KoScriptingOdfReader::namespaceURI() const { return d->currentElement.namespaceURI(); }
int KoScriptingOdfReader::level() const { return d->level; }

QStringList KoScriptingOdfReader::attributeNames() { return d->currentElement.attributeNames(); }
QString KoScriptingOdfReader::attribute(const QString& name, const QString& defaultValue) const { return d->currentElement.attribute(name, defaultValue); }
QString KoScriptingOdfReader::attributeNS(const QString& namespaceURI, const QString& localName, const QString& defaultValue) const { return d->currentElement.attributeNS(namespaceURI, localName, defaultValue); }
bool KoScriptingOdfReader::hasAttribute(const QString& name) const { return d->currentElement.hasAttribute(name); }
bool KoScriptingOdfReader::hasAttributeNS(const QString& namespaceURI, const QString& localName) const { return d->currentElement.hasAttributeNS(namespaceURI, localName); }

bool KoScriptingOdfReader::isNull() const { return d->currentElement.isNull(); }
bool KoScriptingOdfReader::isElement() const { return d->currentElement.isElement(); }
bool KoScriptingOdfReader::isText() const { return d->currentElement.isText(); }
QString KoScriptingOdfReader::text() const { return d->currentElement.text(); }

void KoScriptingOdfReader::emitOnElement() { emit onElement(); }
void KoScriptingOdfReader::setCurrentElement(const KoXmlElement& elem) { d->currentElement = elem; }
void KoScriptingOdfReader::setLevel(int level) { d->level = level; }

void KoScriptingOdfReader::handleElement(KoXmlElement& elem, int level) {
    bool ok = d->filter.isNull();
    if( ! ok ) {
        if( d->filterRegExp.isEmpty() )
            ok = ( d->filter == elem.tagName() );
        else
            ok = d->filterRegExp.exactMatch( elem.tagName() );
    }
    if( ok ) {
        setCurrentElement(elem);
        setLevel(level);
        emitOnElement();
    }
    level++;
    KoXmlElement e;
    forEachElement(e, elem)
        handleElement(e, level); // recursive
}

KoScriptingOdfManifestReader::KoScriptingOdfManifestReader(QObject* parent, const KoXmlDocument& doc) : KoScriptingOdfReader(parent, doc) {
    KoXmlElement elem = doc.documentElement();
    KoXmlElement e;
    forEachElement(e, elem)
        if( e.tagName() == "manifest:file-entry" )
            m_entries << QPair<QString,QString>(e.attribute("manifest:media-type"), e.attribute("manifest:full-path"));
}

QStringList KoScriptingOdfManifestReader::paths(const QString& type) {
    QStringList list;
    for(QList< QPair<QString,QString> >::Iterator it = m_entries.begin(); it != m_entries.end(); ++it)
        if( type.isEmpty() || type == (*it).first )
            list << (*it).second;
    return list;
}

void dumpElem(KoXmlElement elem, int level=0) {
    QString prefix="";
    for(int i = 0; i < level; ++i) prefix+="  ";
    qDebug()<<QString("%1  %2").arg(prefix).arg(elem.tagName());
    foreach(QString s,elem.attributeNames()) qDebug()<<QString("%1    %2 = %3").arg(prefix).arg(s).arg(elem.attribute(s));
    level++;
    KoXmlElement e;
    forEachElement(e, elem) dumpElem(e,level);
}

KoScriptingOdfStylesReader::KoScriptingOdfStylesReader(QObject* parent, const KoXmlDocument& doc) : KoScriptingOdfReader(parent, doc) {
    dumpElem( doc.documentElement() );
}

KoScriptingOdfContentReader::KoScriptingOdfContentReader(QObject* parent, const KoXmlDocument& doc) : KoScriptingOdfReader(parent, doc) {
    dumpElem( doc.documentElement() );
}

/************************************************************************************************
 * KoScriptingOdfStore
 */

/// \internal d-pointer class.
class KoScriptingOdfStore::Private
{
    public:
        QPointer<KoDocument> document;
        QPointer<KoDocumentAdaptor> documentAdaptor;

        KoStore* readStore;
        QIODevice* readDevice;
        KoScriptingOdfReader* reader;
        QByteArray byteArray;

        explicit Private(KoDocument* doc) : document(doc), documentAdaptor(0), readStore(0), readDevice(0), reader(0) {}
        ~Private() { delete readStore; delete readDevice; delete reader; }

        KoStore* getReadStore() {
            QByteArray ba = getByteArray();
            if( ba.isNull() ) {
                qWarning()<<"KoScriptingOdfStore::getReadStore() Failed to fetch ByteArray";
                return 0;
            }
            if( readStore ) {
                //qDebug()<<"KoScriptingOdfStore::getReadStore() Return cached store";
                Q_ASSERT( readDevice );
                return readStore;
            }
            //qDebug()<<"KoScriptingOdfStore::getReadStore() Return new store";
            Q_ASSERT( ! readDevice );
            readDevice = new QBuffer(&byteArray);
            readStore = KoStore::createStore(readDevice, KoStore::Read, "KrossScript", KoStore::Tar);
            return readStore;
        }

        QByteArray getByteArray() {
            if( ! byteArray.isNull() )
                return byteArray;
            if( readStore ) {
                //qDebug()<<"KoScriptingOdfStore::getByteArray() Cleaning prev cached store up.";
                if( readStore->isOpen() ) {
                    //qDebug()<<"KoScriptingOdfStore::getByteArray() Closing prev cached store.";
                    readStore->close();
                }
                delete readStore;
                readStore = 0;
            }
            if( readDevice ) {
                //qDebug()<<"KoScriptingOdfStore::getByteArray() Cleaning prev cached device up.";
                delete readDevice;
                readDevice = 0;
            }
            if( ! document ) {
                //qWarning()<<"KoScriptingOdfStore::getByteArray() No document defined.";
                return QByteArray();
            }

            //qDebug()<<"KoScriptingOdfStore::getByteArray() Reading ByteArray.";
            QBuffer buffer(&byteArray);
            KoStore* store = KoStore::createStore(&buffer, KoStore::Write, "KrossScript", KoStore::Tar);
            KoOasisStore oasisStore(store);
            QByteArray mime = getMimeType();
            KoXmlWriter* manifestWriter = mime.isNull() ? 0 : oasisStore.manifestWriter(mime);
            if( ! document->saveOasis(store, manifestWriter) ) {
                qWarning()<<"KoScriptingOdfStore::open() Failed to save Oasis to ByteArray";
                byteArray = QByteArray();
            }
            //oasisStore.closeContentWriter();
            oasisStore.closeManifestWriter();
            delete store;
            return byteArray;
        }

        QByteArray getMimeType() const {
            return "application/vnd.oasis.opendocument.text"; //odt
            //return "application/vnd.oasis.opendocument.spreadsheet"; //ods
            //return "application/vnd.oasis.opendocument.presentation"; //odp
            //return "pplication/vnd.oasis.opendocument.graphics"; //odg
            //return "application/vnd.oasis.opendocument.chart"; //odc
            //return "application/vnd.oasis.opendocument.formula"; //odf
            //return "application/vnd.oasis.opendocument.image"; //odi
        }

};

KoScriptingOdfStore::KoScriptingOdfStore(QObject* parent, KoDocument* doc)
    : QObject(parent)
    , d(new Private(doc))
{
}

KoScriptingOdfStore::~KoScriptingOdfStore() {
    delete d;
}

bool KoScriptingOdfStore::hasFile(const QString& fileName) {
    KoStore* store = d->getReadStore();
    return store ? store->hasFile(fileName) : false;
}

bool KoScriptingOdfStore::isOpen() const {
    return d->readStore && d->readStore->isOpen();
}

QObject* KoScriptingOdfStore::open(const QString& fileName) {
    delete d->reader; d->reader = 0;
    KoStore* store = d->getReadStore();
    if( ! store )
        return 0;
    if( d->readStore->isOpen() )
        d->readStore->close();
    if( ! store->open(fileName) ) {
        qWarning()<<"KoScriptingOdfStore::openFile() Failed to open file:"<<fileName;
        return 0;
    }
    //qDebug()<<"KoScriptingOdfStore::openFile() fileName="<<fileName<<" store->isOpen="<<store->isOpen()<<endl;
    Q_ASSERT( store->device() );

    //KoOasisStore oasisStore(store);
    KoXmlDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    if( ! doc.setContent(store->device(), &errorMsg, &errorLine, &errorColumn) ) {
        qWarning()<<"KoScriptingOdfStore::openFile() Parse-Error message"<<errorMsg<<"line"<<errorLine<<"col"<<errorColumn;
        return 0;
    }

    const QString tagName = doc.documentElement().tagName();
    qDebug()<<"KoScriptingOdfStore::open documentElement.tagName="<<tagName;
    if( tagName == "office:document-content" )
        d->reader = new KoScriptingOdfContentReader(this, doc);
    if( tagName == "office:document-styles" )
        d->reader = new KoScriptingOdfStylesReader(this, doc);
    else if( tagName == "manifest:manifest" )
        d->reader = new KoScriptingOdfManifestReader(this, doc);
    else
        d->reader = new KoScriptingOdfReader(this, doc);
    return d->reader;
}

bool KoScriptingOdfStore::close() {
    if( ! d->readStore || ! d->readStore->isOpen() )
        return true;
    return d->readStore->close();
}

QObject* KoScriptingOdfStore::document() const {
    if( d->documentAdaptor )
        return d->documentAdaptor;
    return d->document;
}

bool KoScriptingOdfStore::setDocument(QObject* document) {
    //d->clear();
    bool ok = true;
    d->documentAdaptor = dynamic_cast<KoDocumentAdaptor*>(document);
    if( d->documentAdaptor ) {
        d->document = dynamic_cast<KoDocument*>( d->documentAdaptor->parent() );
        Q_ASSERT( d->document );
    }
    else {
        if( KoDocument* doc = dynamic_cast<KoDocument*>(document) ) {
            d->document = doc;
        }
        else {
            d->document = 0;
            ok = false;
        }
        d->documentAdaptor = 0;
    }
    return ok;
}

/*
QString KoScriptingOdfStore::toString() const {
    return QString::fromUtf8( d->byteArray, d->byteArray.length() );
}
*/

#include "KoScriptingOdf.moc"
