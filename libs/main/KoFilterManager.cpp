/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
                 2000, 2001 Werner Trobin <trobin@kde.org>
   Copyright (C) 2004 Nicolas Goutte <goutte@kde.org>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>

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
Boston, MA 02110-1301, USA.
*/

#include "KoFilterManager.h"
#include "KoFilterManager_p.h"
#include "KoDocument.h"
#include "KoDocumentEntry.h"
#include "KoProgressUpdater.h"

#include <QFile>
#include <QLabel>
#include <QVBoxLayout>
#include <QList>
#include <QApplication>
#include <QByteArray>

#include <KLocale>
#include <KMessageBox>
#include <KLibLoader>
#include <KSqueezedTextLabel>
#include <KMimeType>
#include <KDebug>

#include <queue>

#include <unistd.h>

// static cache for filter availability
QMap<QString, bool> KoFilterManager::m_filterAvailable;

KoFilterManager::KoFilterManager(KoDocument* document,
                                 KoProgressUpdater* progressUpdater) :
        m_document(document), m_parentChain(0), m_graph(""),
        d(new Private(progressUpdater))
{
    d->batch = false;
}


KoFilterManager::KoFilterManager(const QString& url, const QByteArray& mimetypeHint,
                                 KoFilterChain* const parentChain) :
        m_document(0), m_parentChain(parentChain), m_importUrl(url), m_importUrlMimetypeHint(mimetypeHint),
        m_graph(""), d(new Private)
{
    d->batch = false;
}

KoFilterManager::KoFilterManager(const QByteArray& mimeType) :
        m_document(0), m_parentChain(0), m_graph(""), d(new Private)
{
    d->batch = false;
    d->importMimeType = mimeType;
}

KoFilterManager::~KoFilterManager()
{
    delete d;
}

QString KoFilterManager::importDocument(const QString& url,
                                        KoFilter::ConversionStatus& status)
{
    // Find the mime type for the file to be imported.
    KUrl u;
    u.setPath(url);
    KMimeType::Ptr t = KMimeType::findByUrl(u, 0, true);
    if (t)
        m_graph.setSourceMimeType(t->name().toLatin1());    // .latin1() is okay here (Werner)
    if (!m_graph.isValid()) {
        bool userCancelled = false;

        kWarning(30500) << "Can't open " << t->name() << ", trying filter chooser";
        if (m_document) {
            if (!m_document->isAutoErrorHandlingEnabled()) {
                status = KoFilter::BadConversionGraph;
                return QString();
            }
            QByteArray nativeFormat = m_document->nativeFormatMimeType();

            QApplication::setOverrideCursor(Qt::ArrowCursor);
            KoFilterChooser chooser(0,
                    KoFilterManager::mimeFilter(nativeFormat, KoFilterManager::Import,
                    m_document->extraNativeMimeTypes(KoDocument::ForImport)), nativeFormat, u);
            if (chooser.exec()) {
                QByteArray f = chooser.filterSelected().toLatin1();

                if (f == nativeFormat) {
                    status = KoFilter::OK;
                    QApplication::restoreOverrideCursor();
                    return url;
                }

                m_graph.setSourceMimeType(f);
            } else
                userCancelled = true;
            QApplication::restoreOverrideCursor();
        }

        if (!m_graph.isValid()) {
            kError(30500) << "Couldn't create a valid graph for this source mimetype: "
                << t->name();
            importErrorHelper(t->name(), userCancelled);
            status = KoFilter::BadConversionGraph;
            return QString();
        }
    }

    KoFilterChain::Ptr chain(0);
    // Are we owned by a KoDocument?
    if (m_document) {
        QByteArray mimeType = m_document->nativeFormatMimeType();
        QStringList extraMimes = m_document->extraNativeMimeTypes(KoDocument::ForImport);
        int i = 0;
        int n = extraMimes.count();
        chain = m_graph.chain(this, mimeType);
        while (i < n) {
            QByteArray extraMime = extraMimes[i].toUtf8();
            // TODO check if its the same target mime then continue
            KoFilterChain::Ptr newChain(0);
            newChain = m_graph.chain(this, extraMime);
            if (!chain || (newChain && newChain->weight() < chain->weight()))
                chain = newChain;
            ++i;
        }
    } else if (!d->importMimeType.isEmpty()) {
        chain = m_graph.chain(this, d->importMimeType);
    } else {
        kError(30500) << "You aren't supposed to use import() from a filter!" << endl;
        status = KoFilter::UsageError;
        return QString();
    }

    if (!chain) {
        kError(30500) << "Couldn't create a valid filter chain!" << endl;
        importErrorHelper(t->name());
        status = KoFilter::BadConversionGraph;
        return QString();
    }

    // Okay, let's invoke the filters one after the other
    m_direction = Import; // vital information!
    m_importUrl = url;  // We want to load that file
    m_exportUrl.clear();  // This is null for sure, as embedded stuff isn't
    // allowed to use that method
    status = chain->invokeChain();

    m_importUrl.clear();  // Reset the import URL

    if (status == KoFilter::OK)
        return chain->chainOutput();
    return QString();
}

KoFilter::ConversionStatus KoFilterManager::exportDocument(const QString& url, QByteArray& mimeType)
{
    bool userCancelled = false;

    // The import url should already be set correctly (null if we have a KoDocument
    // file manager and to the correct URL if we have an embedded manager)
    m_direction = Export; // vital information!
    m_exportUrl = url;

    KoFilterChain::Ptr chain;
    if (m_document) {
        // We have to pick the right native mimetype as source.
        QStringList nativeMimeTypes;
        nativeMimeTypes.append(m_document->nativeFormatMimeType());
        nativeMimeTypes += m_document->extraNativeMimeTypes(KoDocument::ForImport);
        QStringList::ConstIterator it = nativeMimeTypes.constBegin();
        const QStringList::ConstIterator end = nativeMimeTypes.constEnd();
        for (; !chain && it != end; ++it) {
            m_graph.setSourceMimeType((*it).toLatin1());
            if (m_graph.isValid())
                chain = m_graph.chain(this, mimeType);
        }
    } else if (!m_importUrlMimetypeHint.isEmpty()) {
        kDebug(30500) << "Using the mimetype hint:" << m_importUrlMimetypeHint;
        m_graph.setSourceMimeType(m_importUrlMimetypeHint);
    } else {
        KUrl u;
        u.setPath(m_importUrl);
        KMimeType::Ptr t = KMimeType::findByUrl(u, 0, true);
        if (!t || t->name() == KMimeType::defaultMimeType()) {
            kError(30500) << "No mimetype found for" << m_importUrl;
            return KoFilter::BadMimeType;
        }
        m_graph.setSourceMimeType(t->name().toLatin1());

        if (!m_graph.isValid()) {
            kWarning(30500) << "Can't open" << t->name() << ", trying filter chooser";

            QApplication::setOverrideCursor(Qt::ArrowCursor);
            KoFilterChooser chooser(0, KoFilterManager::mimeFilter(), QString(), u);
            if (chooser.exec())
                m_graph.setSourceMimeType(chooser.filterSelected().toLatin1());
            else
                userCancelled = true;

            QApplication::restoreOverrideCursor();
        }
    }

    if (!m_graph.isValid()) {
        kError(30500) << "Couldn't create a valid graph for this source mimetype.";
        if (!userCancelled) KMessageBox::error(0, i18n("Could not export file."), i18n("Missing Export Filter"));
        return KoFilter::BadConversionGraph;
    }

    if (!chain)   // already set when coming from the m_document case
        chain = m_graph.chain(this, mimeType);

    if (!chain) {
        kError(30500) << "Couldn't create a valid filter chain to " << mimeType << " !" << endl;
        KMessageBox::error(0, i18n("Could not export file."), i18n("Missing Export Filter"));
        return KoFilter::BadConversionGraph;
    }

    return chain->invokeChain();
}

namespace  // in order not to mess with the global namespace ;)
{
// This class is needed only for the static mimeFilter method
class Vertex
{
public:
    Vertex(const QByteArray& mimeType) : m_color(White), m_mimeType(mimeType) {}

    enum Color { White, Gray, Black };
    Color color() const {
        return m_color;
    }
    void setColor(Color color) {
        m_color = color;
    }

    QByteArray mimeType() const {
        return m_mimeType;
    }

    void addEdge(Vertex* vertex) {
        if (vertex) m_edges.append(vertex);
    }
    QList<Vertex*> edges() const {
        return m_edges;
    }

private:
    Color m_color;
    QByteArray m_mimeType;
    QList<Vertex*> m_edges;
};

// Some helper methods for the static stuff
// This method builds up the graph in the passed ascii dict
void buildGraph(QHash<QByteArray, Vertex*>& vertices, KoFilterManager::Direction direction)
{
    QStringList stopList; // Lists of mimetypes that are considered end of chains
    stopList << "text/plain";
    stopList << "text/csv";
    stopList << "text/x-tex";
    stopList << "text/html";

    // partly copied from build graph, but I don't see any other
    // way without crude hacks, as we have to obey the direction here
    QList<KoDocumentEntry> parts(KoDocumentEntry::query(QFlag(KoDocumentEntry::AllEntries), QString()));
    QList<KoDocumentEntry>::ConstIterator partIt(parts.constBegin());
    QList<KoDocumentEntry>::ConstIterator partEnd(parts.constEnd());

    while (partIt != partEnd) {
        QStringList nativeMimeTypes = (*partIt).service()->property("X-KDE-ExtraNativeMimeTypes").toStringList();
        nativeMimeTypes += (*partIt).service()->property("X-KDE-NativeMimeType").toString();
        QStringList::ConstIterator it = nativeMimeTypes.constBegin();
        const QStringList::ConstIterator end = nativeMimeTypes.constEnd();
        for (; it != end; ++it)
            if (!(*it).isEmpty()) {
                const QByteArray key = (*it).toLatin1();
                if (!vertices.contains(key))
                    vertices.insert(key, new Vertex(key));
            }
        ++partIt;
    }

    QList<KoFilterEntry::Ptr> filters = KoFilterEntry::query(); // no constraint here - we want *all* :)
    QList<KoFilterEntry::Ptr>::ConstIterator it = filters.constBegin();
    QList<KoFilterEntry::Ptr>::ConstIterator end = filters.constEnd();
    foreach(KoFilterEntry::Ptr filterEntry, filters)
    for (; it != end; ++it) {
        QStringList impList; // Import list
        QStringList expList; // Export list

        // Now we have to exclude the "stop" mimetypes (in the right direction!)
        if (direction == KoFilterManager::Import) {
            // Import: "stop" mime type should not appear in export
            foreach(const QString & testIt, (*it)->export_) {
                if (!stopList.contains(testIt))
                    expList.append(testIt);
            }
            impList = (*it)->import;
        } else {
            // Export: "stop" mime type should not appear in import
            foreach(const QString & testIt, (*it)->import) {
                if (!stopList.contains(testIt))
                    impList.append(testIt);
            }
            expList = (*it)->export_;
        }

        if (impList.empty() || expList.empty()) {
            // This filter cannot be used under these conditions
            kDebug(30500) << "Filter:" << (*it)->service()->name() << " ruled out";
            continue;
        }

        // First add the "starting points" to the dict
        QStringList::ConstIterator importIt = impList.constBegin();
        const QStringList::ConstIterator importEnd = impList.constEnd();
        for (; importIt != importEnd; ++importIt) {
            const QByteArray key = (*importIt).toLatin1();    // latin1 is okay here (werner)
            // already there?
            if (!vertices[ key ])
                vertices.insert(key, new Vertex(key));
        }

        // Are we allowed to use this filter at all?
        if (KoFilterManager::filterAvailable(*it)) {
            QStringList::ConstIterator exportIt = expList.constBegin();
            const QStringList::ConstIterator exportEnd = expList.constEnd();
            for (; exportIt != exportEnd; ++exportIt) {
                // First make sure the export vertex is in place
                const QByteArray key = (*exportIt).toLatin1();    // latin1 is okay here
                Vertex* exp = vertices[ key ];
                if (!exp) {
                    exp = new Vertex(key);
                    vertices.insert(key, exp);
                }
                // Then create the appropriate edges depending on the
                // direction (import/export)
                // This is the chunk of code which actually differs from the
                // graph stuff (apart from the different vertex class)
                importIt = impList.constBegin(); // ### TODO: why only the first one?
                if (direction == KoFilterManager::Import) {
                    for (; importIt != importEnd; ++importIt)
                        exp->addEdge(vertices[(*importIt).toLatin1()]);
                } else {
                    for (; importIt != importEnd; ++importIt)
                        vertices[(*importIt).toLatin1()]->addEdge(exp);
                }
            }
        } else {
            kDebug(30500) << "Filter:" << (*it)->service()->name() << " does not apply.";
        }
    }
}

// This method runs a BFS on the graph to determine the connected
// nodes. Make sure that the graph is "cleared" (the colors of the
// nodes are all white)
QStringList connected(const QHash<QByteArray, Vertex*>& vertices, const QByteArray& mimetype)
{
    if (mimetype.isEmpty())
        return QStringList();
    Vertex *v = vertices[ mimetype ];
    if (!v)
        return QStringList();

    v->setColor(Vertex::Gray);
    std::queue<Vertex*> queue;
    queue.push(v);
    QStringList connected;

    while (!queue.empty()) {
        v = queue.front();
        queue.pop();
        QList<Vertex*> edges = v->edges();
        foreach(Vertex* current, edges) {
            if (current->color() == Vertex::White) {
                current->setColor(Vertex::Gray);
                queue.push(current);
            }
        }
        v->setColor(Vertex::Black);
        connected.append(v->mimeType());
    }
    return connected;
}
} // anon namespace

// The static method to figure out to which parts of the
// graph this mimetype has a connection to.
QStringList KoFilterManager::mimeFilter(const QByteArray &mimetype, Direction direction, const QStringList &extraNativeMimeTypes)
{
    //kDebug(30500) <<"mimetype=" << mimetype <<" extraNativeMimeTypes=" << extraNativeMimeTypes;
    QHash<QByteArray, Vertex*> vertices;
    buildGraph(vertices, direction);

    // TODO maybe use the fake vertex trick from the method below, to make the search faster?

    QStringList nativeMimeTypes;
    nativeMimeTypes.append(QString::fromLatin1(mimetype));
    nativeMimeTypes += extraNativeMimeTypes;

    // Add the native mimetypes first so that they are on top.
    QStringList lst = nativeMimeTypes;

    // Now look for filters which output each of those natives mimetypes
    foreach(const QString &natit, nativeMimeTypes) {
        const QStringList outMimes = connected(vertices, natit.toLatin1());
        //kDebug(30500) <<"output formats connected to mime" << natit <<" :" << outMimes;
        foreach(const QString &mit, outMimes) {
            if (!lst.contains(mit))     // append only if not there already. Qt4: QSet<QString>?
                lst.append(mit);
        }
    }
    foreach(Vertex* vertex, vertices) {
        delete vertex;
    }
    vertices.clear();
    return lst;
}

QStringList KoFilterManager::mimeFilter()
{
    QHash<QByteArray, Vertex*> vertices;
    buildGraph(vertices, KoFilterManager::Import);

    QList<KoDocumentEntry> parts(KoDocumentEntry::query(QFlag(KoDocumentEntry::AllEntries), QString()));
    QList<KoDocumentEntry>::ConstIterator partIt(parts.constBegin());
    QList<KoDocumentEntry>::ConstIterator partEnd(parts.constEnd());

    if (partIt == partEnd)
        return QStringList();

    // To find *all* reachable mimetypes, we have to resort to
    // a small hat trick, in order to avoid multiple searches:
    // We introduce a fake vertex, which is connected to every
    // single KOffice mimetype. Due to that one BFS is enough :)
    // Now we just need an... ehrm.. unique name for our fake mimetype
    Vertex *v = new Vertex("supercalifragilistic/x-pialadocious");
    vertices.insert("supercalifragilistic/x-pialadocious", v);
    while (partIt != partEnd) {
        QStringList nativeMimeTypes = (*partIt).service()->property("X-KDE-ExtraNativeMimeTypes").toStringList();
        nativeMimeTypes += (*partIt).service()->property("X-KDE-NativeMimeType").toString();
        QStringList::ConstIterator it = nativeMimeTypes.constBegin();
        const QStringList::ConstIterator end = nativeMimeTypes.constEnd();
        for (; it != end; ++it)
            if (!(*it).isEmpty())
                v->addEdge(vertices[(*it).toLatin1()]);
        ++partIt;
    }
    QStringList result = connected(vertices, "supercalifragilistic/x-pialadocious");

    // Finally we have to get rid of our fake mimetype again
    result.removeAll("supercalifragilistic/x-pialadocious");
    return result;
}

// Here we check whether the filter is available. This stuff is quite slow,
// but I don't see any other convenient (for the user) way out :}
bool KoFilterManager::filterAvailable(KoFilterEntry::Ptr entry)
{
    if (!entry)
        return false;
    if (entry->available != "check")
        return true;

    //kDebug( 30500 ) <<"Checking whether" << entry->service()->name() <<" applies.";
    // generate some "unique" key
    QString key(entry->service()->name());
    key += " - ";
    key += entry->service()->library();

    if (!m_filterAvailable.contains(key)) {
        //kDebug( 30500 ) <<"Not cached, checking...";

        KLibrary* library = KLibLoader::self()->library(QFile::encodeName(entry->service()->library()));
        if (!library) {
            kWarning(30500) << "Huh?? Couldn't load the lib: "
                << KLibLoader::self()->lastErrorMessage() << endl;
            m_filterAvailable[ key ] = false;
            return false;
        }

        // This code is "borrowed" from klibloader ;)
        QByteArray symname = "check_" + QString(library->objectName()).toLatin1();
        KLibrary::void_function_ptr sym = library->resolveFunction(symname);
        if (!sym) {
            kWarning(30500) << "The library " << library->objectName()
                << " does not offer a check_" << library->objectName()
                << " function." << endl;
            m_filterAvailable[key] = false;
        } else {
            typedef int (*t_func)();
            t_func check = (t_func)sym;
            m_filterAvailable[ key ] = check() == 1;
        }
    }
    return m_filterAvailable[key];
}

void KoFilterManager::importErrorHelper(const QString& mimeType, const bool suppressDialog)
{
    QString tmp = i18n("Could not import file of type\n%1", mimeType);
    // ###### FIXME: use KLibLoader::lastErrorMessage() here
    if (!suppressDialog) KMessageBox::error(0, tmp, i18n("Missing Import Filter"));
}

void KoFilterManager::setBatchMode(const bool batch)
{
    d->batch = batch;
}

bool KoFilterManager::getBatchMode(void) const
{
    return d->batch;
}

KoProgressUpdater* KoFilterManager::progressUpdater() const
{
    return d->progressUpdater;
}

#include <KoFilterManager.moc>
