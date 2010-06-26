/***************************************************************************
  KoScriptingOdf.cpp
 * This file is part of the KDE project
 * copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "KoScriptingOdf.h"

#include <KoStore.h>
#include <KoOdfWriteStore.h>
#include <KoDocumentAdaptor.h>
#include <KoEmbeddedDocumentSaver.h>

#include <KDebug>
#include <QBuffer>

/************************************************************************************************
 * KoScriptingOdfReader
 */

KoScriptingOdfReader::KoScriptingOdfReader(KoScriptingOdfStore *store, const KoXmlDocument &doc)
    : QObject(store),
    m_store(store),
    m_doc(doc),
    m_level(0)
{
}

KoScriptingOdfReader::~KoScriptingOdfReader()
{
}

void KoScriptingOdfReader::start()
{
    KoXmlElement elem = m_doc.documentElement();
    handleElement(elem);
    setCurrentElement(KoXmlElement());
    setLevel(0);
}

QString KoScriptingOdfReader::nameFilter() const
{
    return m_filter;
}

void KoScriptingOdfReader::setNameFilter(const QString &name, bool regularExpression)
{
    m_filter = name.isEmpty() ? QString() : name;
    m_filterRegExp = regularExpression ? QRegExp(name, Qt::CaseInsensitive) : QRegExp();
}

KoScriptingOdfStore *KoScriptingOdfReader::store() const
{
    return m_store;
}

KoXmlDocument KoScriptingOdfReader::doc() const
{
    return m_doc;
}

KoXmlElement KoScriptingOdfReader::currentElement() const
{
    return m_currentElement;
}

QString KoScriptingOdfReader::name() const
{
    return m_currentElement.tagName(); /*.nodeName();*/
}

QString KoScriptingOdfReader::namespaceURI() const
{
    return m_currentElement.namespaceURI();
}

int KoScriptingOdfReader::level() const
{
    return m_level;
}

#ifndef KOXML_USE_QDOM
QStringList KoScriptingOdfReader::attributeNames()
{
    return m_currentElement.attributeNames();
}
#endif

QString KoScriptingOdfReader::attribute(const QString &name, const QString &defaultValue) const
{
    return m_currentElement.attribute(name, defaultValue);
}

QString KoScriptingOdfReader::attributeNS(const QString &namespaceURI, const QString &localName, const QString &defaultValue) const
{
    return m_currentElement.attributeNS(namespaceURI, localName, defaultValue);
}

bool KoScriptingOdfReader::hasAttribute(const QString &name) const
{
    return m_currentElement.hasAttribute(name);
}

bool KoScriptingOdfReader::hasAttributeNS(const QString &namespaceURI, const QString &localName) const
{
    return m_currentElement.hasAttributeNS(namespaceURI, localName);
}

bool KoScriptingOdfReader::isNull() const
{
    return m_currentElement.isNull();
}

bool KoScriptingOdfReader::isElement() const
{
    return m_currentElement.isElement();
}

QString KoScriptingOdfReader::text() const
{
    return m_currentElement.text();
}

bool KoScriptingOdfReader::hasChildren() const {
#ifdef KOXML_USE_QDOM
    const int count = m_currentElement.childNodes().count();
#else
    const int count = m_currentElement.childNodesCount();
#endif
    if (count < 1)
        return false;
    if (count == 1 && m_currentElement.firstChild().isText())
        return false;
    return true;
}

void KoScriptingOdfReader::emitOnElement()
{
emit onElement();
}

void KoScriptingOdfReader::setCurrentElement(const KoXmlElement &elem)
{
    m_currentElement = elem;
}

void KoScriptingOdfReader::setLevel(int level)
{
    m_level = level;
}

void KoScriptingOdfReader::handleElement(KoXmlElement &elem, int level)
{
    bool ok = m_filter.isNull();
    if (! ok) {
        if (m_filterRegExp.isEmpty())
            ok = m_filter == elem.tagName();
        else
            ok = m_filterRegExp.exactMatch(elem.tagName());
    }
    if (ok) {
        setCurrentElement(elem);
        setLevel(level);
        emitOnElement();
    }
    level++;
    KoXmlElement e;
    forEachElement(e, elem)
        handleElement(e, level); // recursive
}

KoScriptingOdfManifestReader::KoScriptingOdfManifestReader(KoScriptingOdfStore *store, const KoXmlDocument &doc)
    : KoScriptingOdfReader(store, doc)
{
    KoXmlElement elem = doc.documentElement();
    KoXmlElement e;
    forEachElement(e, elem)
        if (e.tagName() == "manifest:file-entry")
            m_entries << QPair<QString,QString>(e.attribute("manifest:media-type"), e.attribute("manifest:full-path"));
}

QStringList KoScriptingOdfManifestReader::paths(const QString &type)
{
    QStringList list;
    for (QList<QPair<QString,QString> >::Iterator it = m_entries.begin(); it != m_entries.end(); ++it)
        if (type.isEmpty() || type == (*it).first )
            list << (*it).second;
    return list;
}

#ifndef NDEBUG
void dumpElem(KoXmlElement elem, int level=0)
{
    QString prefix;
    for (int i = 0; i < level; ++i)
        prefix+="  ";
    kDebug(32010)  << QString("%1  %2").arg(prefix).arg(elem.tagName());
#ifndef KOXML_USE_QDOM
    foreach (const QString &s, elem.attributeNames())
        kDebug(32010)  << QString("%1    %2 = %3").arg(prefix).arg(s).arg(elem.attribute(s));
#endif
    level++;
    KoXmlElement e;
    forEachElement(e, elem)
        dumpElem(e,level);
}
#endif

KoScriptingOdfStylesReader::KoScriptingOdfStylesReader(KoScriptingOdfStore *store, const KoXmlDocument &doc)
    : KoScriptingOdfReader(store, doc)
{
    //dumpElem( doc.documentElement() );
}

KoScriptingOdfContentReader::KoScriptingOdfContentReader(KoScriptingOdfStore *store, const KoXmlDocument &doc)
    : KoScriptingOdfReader(store, doc)
{
    //dumpElem( doc.documentElement() );
}

/************************************************************************************************
 * KoScriptingOdfStore
 */

KoStore *KoScriptingOdfStore::getReadStore()
{
    QByteArray ba = getByteArray();
    if (ba.isNull()) {
        kWarning(32010)  << "KoScriptingOdfStore::getReadStore() Failed to fetch ByteArray";
        return 0;
    }
    if (m_readStore ) {
        //kDebug(32010) <<"KoScriptingOdfStore::getReadStore() Return cached store";
        Q_ASSERT(m_readDevice);
        return m_readStore;
    }
    //kDebug(32010) <<"KoScriptingOdfStore::getReadStore() Return new store";
    Q_ASSERT(!m_readDevice);
    m_readDevice = new QBuffer(&m_byteArray);
    m_readStore = KoStore::createStore(m_readDevice, KoStore::Read, "KrossScript", KoStore::Tar);
    return m_readStore;
}

QByteArray KoScriptingOdfStore::getByteArray()
{
    if (! m_byteArray.isNull())
        return m_byteArray;
    if (m_readStore) {
        //kDebug(32010)  << "KoScriptingOdfStore::getByteArray() Cleaning prev cached store up.";
        if (m_readStore->isOpen() ) {
            //kDebug(32010)  << "KoScriptingOdfStore::getByteArray() Closing prev cached store.";
            m_readStore->close();
        }
        delete m_readStore;
        m_readStore = 0;
    }
    if (m_readDevice) {
        //kDebug(32010)  << "KoScriptingOdfStore::getByteArray() Cleaning prev cached device up.";
        delete m_readDevice;
        m_readDevice = 0;
    }
    if (! m_document) {
        //kWarning(32010)  << "KoScriptingOdfStore::getByteArray() No document defined.";
        return QByteArray();
    }

    //kDebug(32010)  << "KoScriptingOdfStore::getByteArray() Reading ByteArray.";
    QBuffer buffer(&m_byteArray);
    KoStore *store = KoStore::createStore(&buffer, KoStore::Write, "KrossScript", KoStore::Tar);
    KoOdfWriteStore odfStore(store);
    odfStore.manifestWriter("");
    KoEmbeddedDocumentSaver embeddedSaver;
    KoDocument::SavingContext documentContext(odfStore, embeddedSaver);
    QByteArray mime = getMimeType();
    if (! m_document->saveOdf(documentContext)) {
        kWarning(32010)  << "KoScriptingOdfStore::open() Failed to save Oasis to ByteArray";
        m_byteArray = QByteArray();
    }
    //odfStore.closeContentWriter();
    odfStore.closeManifestWriter();
    delete store;
    return m_byteArray;
}

QByteArray KoScriptingOdfStore::getMimeType() const
{
    return "application/vnd.oasis.opendocument.text"; //odt
    //return "application/vnd.oasis.opendocument.spreadsheet"; //ods
    //return "application/vnd.oasis.opendocument.presentation"; //odp
    //return "pplication/vnd.oasis.opendocument.graphics"; //odg
    //return "application/vnd.oasis.opendocument.chart"; //odc
    //return "application/vnd.oasis.opendocument.formula"; //odf
    //return "application/vnd.oasis.opendocument.image"; //odi
}

KoScriptingOdfStore::KoScriptingOdfStore(QObject *parent, KoDocument *doc)
    : QObject(parent),
    m_document(doc),
    m_documentAdaptor(0),
    m_readStore(0),
    m_readDevice(0),
    m_reader(0)
{
}

KoScriptingOdfStore::~KoScriptingOdfStore()
{
    delete m_readStore;
    delete m_readDevice;
    delete m_reader;
}

//KoStore *KoScriptingOdfStore::readStore() const { return getReadStore(); }
//QIODevice *KoScriptingOdfStore::readDevice() const { return readDevice; }

bool KoScriptingOdfStore::hasFile(const QString &fileName)
{
    KoStore *store = getReadStore();
    return store ? store->hasFile(fileName) : false;
}

bool KoScriptingOdfStore::isOpen() const
{
    return m_readStore && m_readStore->isOpen();
}

QObject *KoScriptingOdfStore::open(const QString &fileName)
{
    delete m_reader; m_reader = 0;
    KoStore *store = getReadStore();
    if (! store)
        return 0;
    if (store->isOpen())
        store->close();
    if (! store->open(fileName)) {
        kWarning(32010) <<"KoScriptingOdfStore::openFile() Failed to open file:"<<fileName;
        return 0;
    }
    //kDebug(32010) <<"KoScriptingOdfStore::openFile() fileName="<<fileName<<" store->isOpen="<<store->isOpen()<<endl;
    Q_ASSERT(store->device());

    //KoOasisStore oasisStore(store);
    KoXmlDocument doc;

    QString errorMsg;
    int errorLine, errorColumn;
    if (! doc.setContent(store->device(), &errorMsg, &errorLine, &errorColumn)) {
       kWarning(32010) << "Parse-Error message" << errorMsg << "line" << errorLine << "col" << errorColumn;
       return 0;
    }

    const QString tagName = doc.documentElement().tagName();
    kDebug(32010) <<"KoScriptingOdfStore::open documentElement.tagName="<<tagName;
    if (tagName == "office:document-content")
        m_reader = new KoScriptingOdfContentReader(this, doc);
    if (tagName == "office:document-styles")
        m_reader = new KoScriptingOdfStylesReader(this, doc);
    else if (tagName == "manifest:manifest")
        m_reader = new KoScriptingOdfManifestReader(this, doc);
    else
        m_reader = new KoScriptingOdfReader(this, doc);
    return m_reader;
}

bool KoScriptingOdfStore::close()
{
    if (! m_readStore || ! m_readStore->isOpen())
        return true;
    return m_readStore->close();
}

QByteArray KoScriptingOdfStore::extract(const QString &fileName)
{
    KoStore *store = getReadStore();
    if (! store)
        return QByteArray();
    if (store->isOpen())
        store->close();
    QByteArray data;
    bool ok = store->extractFile(fileName, data);
    return ok ? data : QByteArray();
}

bool KoScriptingOdfStore::extractToFile(const QString &fileName, const QString &toFileName)
{
    KoStore *store = getReadStore();
    if (! store)
        return false;
    if (store->isOpen())
        store->close();
    return store->extractFile(fileName, toFileName);
}

QObject *KoScriptingOdfStore::document() const
{
    if (m_documentAdaptor)
        return m_documentAdaptor;
    return m_document;
}

bool KoScriptingOdfStore::setDocument(QObject *document)
{
    bool ok = true;
    m_documentAdaptor = dynamic_cast<KoDocumentAdaptor*>(document);
    if (m_documentAdaptor) {
        m_document = dynamic_cast<KoDocument*>(m_documentAdaptor->parent());
        Q_ASSERT(m_document);
    } else {
        if (KoDocument *doc = dynamic_cast<KoDocument*>(document)) {
            m_document = doc;
        } else {
            m_document = 0;
            ok = false;
        }
        m_documentAdaptor = 0;
    }
    return ok;
}

#include <KoScriptingOdf.moc>
